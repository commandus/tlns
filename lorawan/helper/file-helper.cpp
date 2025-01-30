#if defined(_MSC_VER) || defined(__MINGW32__)
#pragma warning(disable: 4996)
#include <windows.h>
#include <io.h>
#include <cwchar>
#include <cstdio>
#include <shlobj.h>
#define PATH_DELIMITER "\\"
#else
#include <sys/param.h>
#include <fcntl.h>
#include <ftw.h>
#include <pwd.h>

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <string.h>
#include <errno.h>
#include <cstdio>

#define PATH_DELIMITER "/"

#ifndef F_GETPATH
#define F_GETPATH	(1024 + 7)
#endif

#endif

#include <iostream>

#include "file-helper.h"

#ifdef __ANDROID__
#if __ANDROID_API__ < 17
#error Android API must be 17 or more for ftw()
#endif

#if __ANDROID_API__ < 21
#error Android API must be 17 or more for fts_open()
#endif

#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
/**
* A FILETIME is the number of 100-nanosecond intervals since January 1, 1601.
* A time_t is the number of 1-second intervals since January 1, 1970.
* 116444736000000000
* @link https://www.gamedev.net/forums/topic/565693-converting-filetime-to-time_t-on-windows/
*/
static time_t filetime2time_t(
    FILETIME const& ft
)
{
    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;
    return (ull.QuadPart / 10000000ULL) - 11644473600ULL;
}

bool file::isOrdinalFile(
    time_t& retModificationTime,
    const char* path
) {
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    GetFileAttributesExA(path, GetFileExInfoStandard, (void*) &fileInfo);
    retModificationTime = filetime2time_t(fileInfo.ftLastWriteTime);
    if (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        return false;
    return true;
}

bool file::rmAllDir(const char *path)
{
	if (!path)
		return false;
	size_t sz = strlen(path);
	if (sz <= 1)
		return false;	// prevent "rm -r /"
	char fp[MAX_PATH];
	memmove(fp, path, sz);
	fp[sz] = '\0';
	fp[sz + 1] = '\0';
	SHFILEOPSTRUCTA shfo = {
		nullptr,
		FO_DELETE,
		fp,
		nullptr,
		FOF_SILENT | FOF_NOERRORUI | FOF_NOCONFIRMATION,
		FALSE,
		nullptr,
		nullptr };

	SHFileOperationA(&shfo);
    return true;
}

bool file::mkDir(
    const std::string &path
)
{
    return CreateDirectoryA(path.c_str(), nullptr);
}

bool file::rmDir(
    const std::string &path
)
{
	if (&path == nullptr)
		return false;
	if (path.size() <= 1)
		return false;	// prevent "rm -r /"
	const char *sDir = path.c_str();
	WIN32_FIND_DATAA fdFile;
	HANDLE hFind;
	char sPath[MAX_PATH];
	sprintf(sPath, "%s\\*.*", sDir);
	if ((hFind = FindFirstFileA(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
		return false;
	do
	{
		if (strcmp(fdFile.cFileName, ".") != 0 && strcmp(fdFile.cFileName, "..") != 0)
		{
			sprintf(sPath, "%s\\%s", sDir, fdFile.cFileName);
			if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				// Is Directory
				rmAllDir(sPath);
			}
		}
	} while (FindNextFileA(hFind, &fdFile));
	FindClose(hFind);
	return true;
}

/**
 * Return list of files in specified paths
 * @param path
 * @param retval can be NULL
 * @return count files
 */
size_t file::filesInPath
(
	const std::string &aPath,
	const std::string &suffix,
	int flags,
	std::vector<std::string> *retval
)
{
	std::string path;
	std::string search_path;

	if (aPath.size() > 3 && aPath[1] == ':' && aPath[2] =='\\')
		path = aPath;
	else
		path = getCurrentDir() + "\\" + aPath;
	search_path = path + "\\*.*";
		
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
	size_t r = 0;
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			std::string f(fd.cFileName);
			if ((f == ".") || (f == ".."))
				continue;
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				r += filesInPath(f, suffix, flags, retval);
			} else {
				if (f.find(suffix) != std::string::npos) {
					if (retval) {
						switch (flags) {
						case 1:
						{
							std::string s(path + "\\" + fd.cFileName);
							retval->push_back(s);

						}
						break;
						default:
							retval->push_back(fd.cFileName);
						}
					}
					r++;
				}
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return r;
}

#define F_OK	0

std::string file::expandFileName(
    const std::string &relativeName
)
{
    char realPath[MAX_PATH + 1];
    GetFullPathName(relativeName.c_str(), MAX_PATH, realPath, nullptr);
    return std::string(realPath);
}

bool file::fileExists(const std::string &fileName)
{
    return _access(fileName.c_str(), F_OK) != -1;
}

#else

/**
* FTW_D	directory
* FTW_DNR	directory that cannot be read
* FTW_F	file
* FTW_SL	symbolic link
* FTW_NS	other than a symbolic link on which stat() could not successfully be executed.
*/
static int rmnode
(
	const char *path,
	const struct stat *ptr,
	int flag,
	struct FTW *ftwbuf
)
{
	int(*rm_func)(const char *);

	switch (flag)
	{
	case FTW_D:
	case FTW_DP:
		rm_func = rmdir;
		break;
	default:
		rm_func = unlink;
		break;
	}
	rm_func(path);
	return 0;
}


bool file::mkDir(
    const std::string &path
)
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        // Directory does not exist. EEXIST for race condition
        if (mkdir(path.c_str(), 0777) != 0 && errno != EEXIST)
            return false;
    } else if (!S_ISDIR(st.st_mode)) {
        return false;
    }
    return true;
}

bool file::rmDir(const std::string &path)
{
	if (path.size() <= 1)
		return false;	// prevent "rm -r /"
	return nftw(path.c_str(), rmnode,  64, FTW_DEPTH) == 0;
}

static int compareFile
(
		const FTSENT **a,
		const FTSENT **b
)
{
	return strcmp((*a)->fts_name, (*b)->fts_name);
}

/**
 * Return list of files in specified path
 * @param path
 * @param flags 0- as is, 1- full path, 2- relative (remove parent path)
 * @param retval can be NULL
 * @return count files
 * FreeBSD fts.h fts_*()
 */
size_t file::filesInPath
(
	const std::string &path,
	const std::string &suffix,
	int flags,
	std::vector<std::string> *retval
)
{
	char *pathList[2];
    pathList[1] = nullptr;
	if (flags & 1) {
		char realPath[PATH_MAX + 1];
        pathList[0] = realpath((char *) path.c_str(), realPath);
	} else {
        pathList[0] = (char *) path.c_str();
	}
    if (!pathList[0])
        return 0;

	int parent_len = strlen(pathList[0]) + 1;	///< Arggh. Remove '/' path delimiter(I mean it 'always' present). Not sure is it works fine. It's bad, I know.

	FTS* file_system = fts_open(pathList, FTS_LOGICAL | FTS_NOSTAT, nullptr);

    if (!file_system)
    	return 0;
    size_t count = 0;
	while(FTSENT* parent = fts_read(file_system)) {
		FTSENT* child = fts_children(file_system, 0);
		if (errno != 0) {
			// ignore, perhaps permission error
		}
		while (child)
		{
			switch (child->fts_info) {
				case FTS_F:
					{
						std::string s(child->fts_name);
						if (s.find(suffix) != std::string::npos)
						{
							count++;
							if (retval)
							{
								if (flags & 2)
								{
									// extract parent path
									std::string p(&child->fts_path[parent_len]);
									retval->push_back(p + s);
								}
								else
									retval->push_back(std::string(child->fts_path) + s);
							}
						}
					}
					break;
				default:
					break;
			}
			child = child->fts_link;
		}
	}
	fts_close(file_system);
	return count;
}

bool file::fileExists(
    const std::string &fileName
)
{
    struct stat r;
    return stat(fileName.c_str(), &r) == 0;
}

std::string file::expandFileName(
    const std::string &relativeName
)
{
    char realPath[PATH_MAX + 1];
    auto r = realpath((char *) relativeName.c_str(), realPath);
    if (r)
        return std::string(r);
    return relativeName;
}

bool file::isOrdinalFile(
	time_t &retModificationTime,
	const char *path
) {
	struct stat s;
	if (stat(path, &s) == 0 ) {
		retModificationTime = s.st_mtime;
		if (s.st_mode & S_IFREG )
			return true;
	}
	return false;
}

#endif

bool file::rmFile(const std::string &fn)
{
	return std::remove((const char*) fn.c_str()) == 0;
}

/**
 * Return true if file name extension is .json
 * @param path file name to examine
 * @return true if file name extension is .json
 */
bool file::fileIsJSON(
	const std::string &path
)
{
	return (path.find(".json") == path.size() - 5);
}

/**
 * @return last modification file time, seconds since unix epoch
 */
time_t fileModificationTime(
	const std::string &fileName
)
{
#if defined(_MSC_VER) || defined(__MINGW32__)
	WIN32_FILE_ATTRIBUTE_DATA fileInfo;
	GetFileAttributesEx(fileName.c_str(), GetFileExInfoStandard, (void *)&fileInfo);
	return filetime2time_t(fileInfo.ftLastWriteTime);
#else
	struct stat attrib;
	stat(fileName.c_str(), &attrib);
	return attrib.st_mtime;
#endif	
}

URL::URL(const std::string &url)
{
    parse(url);
}

void URL::clear() {
    protocol = "";
    host = "";
    path = "";
    query = "";
}

void URL::parse(
    const std::string &url
)
{
    clear();
    size_t pHost;
    size_t pProto = url.find("://");
    if (pProto != std::string::npos) {
        protocol = url.substr(0, pProto);
        pProto += 3;
    } else
        pProto = 0;

    pHost = url.find('/', pProto);
    if (pHost != std::string::npos) {
        host = url.substr(pProto, pHost - pProto);
    } else
        pHost = pProto;

    size_t pPath = url.find('?', pHost);
    if (pPath == std::string::npos) {
        pPath = url.size();
    }
    path = url.substr(pHost, pPath - pHost);
    pPath++;

    if (pPath >= url.size())
        return;
    query = url.substr(pPath);
}

std::string URL::get(
    const std::string &name
)
{
    size_t p = 0;
    while(true) {
        p = query.find(name, p);
        if (p == std::string::npos)
            return "";
        size_t pEq = p + name.size();
        if (pEq >= query.size())
            return "";
        pEq++;
        if (query[pEq] != '=') {
            p = pEq;
            continue;
        }
        pEq++;
        if (pEq >= query.size())
            return "";
        size_t pEnd = query.find('&', pEq);
        if (pEnd == std::string::npos)
            pEnd = query.size() + 1;
        return query.substr(pEq, pEnd - pEq);
    }
}

int URL::getInt(
    const std::string &name
)
{
    std::string s = get(name);
    if (s.empty())
        return 0;
    return (int) strtoll(s.c_str(), nullptr, 10);
}

std::string getCurrentDir()
{
#if defined(_MSC_VER) || defined(__MINGW32__)
    TCHAR buffer[MAX_PATH];
    GetCurrentDirectory(MAX_PATH - 1, buffer);
    return std::string((char *) buffer);
#else
    char wd[PATH_MAX];
    return getcwd(wd, PATH_MAX);
#endif
}

std::string getHomeDir()
{
#if defined(_MSC_VER) || defined(__MINGW32__)
	CHAR path[MAX_PATH];
	HRESULT result = SHGetFolderPathA(nullptr, CSIDL_PROFILE, nullptr, 0, path);
	if (!SUCCEEDED(result))
		return "";
	return std::string(path);
#else
	struct passwd *pw = getpwuid(getuid());
	return std::string(pw->pw_dir);
#endif
}

std::string getProgramDir()
{
#if defined(_MSC_VER) || defined(__MINGW32__)
    // Windows
    CHAR path[MAX_PATH];
    HRESULT result = GetModuleFileNameA(nullptr,path,MAX_PATH);
    if (!SUCCEEDED(result))
        return "";
    return std::string(path);
#else
    // Linux
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    return std::string(path, (count > 0) ? count : 0);
#endif
}
