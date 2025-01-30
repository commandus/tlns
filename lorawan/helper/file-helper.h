/*
 * @file file-helper.h
 */
#ifndef FILE_HELPER_H
#define FILE_HELPER_H     1

#include <string>
#include <vector>

/**
 * File name helper functions.
 * Do not forget namespace file::
 */
namespace file {
	bool rmDir(const std::string &path);
	bool rmFile(const std::string &fn);
#if defined(_MSC_VER) || defined(__MINGW32__)
	bool rmAllDir(const char* path);
#endif
	/**
	 * Return list of files in specified path
	 * @param path
	 * @param flags 0- as is, 1- full path, 2- relative (remove parent path)
	 * @param retval can be NULL
	 * @return count files
	 * FreeBSD fts.h fts_*()
	 */
	size_t filesInPath(
		const std::string &path,
		const std::string &suffix,
		int flags,
		std::vector<std::string> *retval
	);

    bool fileExists(const std::string &fileName);
    std::string expandFileName(const std::string &relativeName);

	/**
	 * @brief Return is file ordinal. If not, it is directory or special file.
	 *
	 * @param reModificationTime return modofication time
	 * @param path directory or file path
	 * @return true - ordinal file
	 * @return false - directory ot special file
	 */
	bool isOrdinalFile(
		time_t &reModificationTime,
		const char *path
	);

	/**
	 * Return true if file name extension is .json
	 * @param path file name to examine
	 * @return true if file name extension is .json
	 */
	bool fileIsJSON(const std::string &path);

}

/**
 * @return last modification file time, seconds since unix epoch
 */
time_t fileModificationTime(
	const std::string &fileName
);

class URL {
private:
    void parse(const std::string &url);
    void clear();
public:
    std::string protocol;
    std::string host;
    std::string path;
    std::string query;
    URL(const std::string &url);
    // getUplink query parameter value (first one)
    std::string get(const std::string &name);
    // getUplink query parameter value as integer (first one)
    int getInt(const std::string &name);
};

std::string getCurrentDir();
std::string getHomeDir();
std::string getProgramDir();

#endif
