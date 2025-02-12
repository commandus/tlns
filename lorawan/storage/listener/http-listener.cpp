#include <cstring>
#include <iostream>

#include <microhttpd.h>

#include "http-listener.h"

#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-error.h"

#include <sys/stat.h>
#include <sstream>
#include <algorithm>

#if defined(_MSC_VER) || defined(__MINGW32__)
#pragma warning(disable: 4996)
#endif

#ifdef ENABLE_QRCODE
#include "nayuki/qrcodegen.hpp"
#include "lorawan/storage/serialization/qr-helper.h"
#include "lorawan/storage/serialization/urn-helper.h"
#endif

#define MHD_START_FLAGS 	(MHD_USE_POLL | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_SUPPRESS_DATE_NO_CLOCK | MHD_USE_TCP_FASTOPEN | MHD_USE_TURBO)
#define DEF_HTML_INDEX_FILE_NAME "index.html"

const static char *CE_GZIP = "gzip";
const static char *CT_HTML = "text/html;charset=UTF-8";
const static char *CT_JSON = "text/javascript;charset=UTF-8";
const static char *CT_KML = "application/vnd.google-earth.kml+xml";
const static char *CT_PNG = "image/png";
const static char *CT_JPEG = "image/jpeg";
const static char *CT_SVG = "image/svg+xml";
const static char *CT_CSS = "text/css";
const static char *CT_TEXT = "text/plain;charset=UTF-8";
const static char *CT_TTF = "font/ttf";
const static char *CT_BIN = "application/octet";

// Caution: version may be different, if microhttpd dependency not compiled, revise version humber
#if MHD_VERSION <= 0x00096600
#define MHD_Result int
#endif
#ifndef MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_CREDENTIALS
#define MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_CREDENTIALS "Access-Control-Allow-Credentials"
#endif
#ifndef MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_METHODS
#define MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_METHODS "Access-Control-Allow-Methods"
#endif
#ifndef MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_HEADERS
#define MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_HEADERS "Access-Control-Allow-Headers"
#endif
#ifndef MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN
#define MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN "Access-Control-Allow-Origin"
#endif

#define DEF_HTTP_PORT 4248

HTTPListener::HTTPListener(
    IdentitySerialization* aIdentitySerialization,
    GatewaySerialization* aSerializationWrapper,
    const std::string &aHTMLRootDir
)
    : StorageListener(aIdentitySerialization, aSerializationWrapper),
      port(DEF_HTTP_PORT), log(nullptr), verbose(0), flags(MHD_START_FLAGS),
      threadCount(1), connectionLimit(32768), descriptor(nullptr),
      mimeType(aIdentitySerialization ? aIdentitySerialization->mimeType() : serializationKnownType2MimeType(SKT_BINARY)),
      htmlRootDir(aHTMLRootDir)
{
}

HTTPListener::~HTTPListener()
{
    stop();
}

void HTTPListener::setLog(
    int aVerbose,
    Log* aLog
)
{
    verbose = aVerbose;
    log = aLog;
}

void HTTPListener::stop()
{
    if (descriptor) {
        MHD_stop_daemon((struct MHD_Daemon *) descriptor);
        descriptor = nullptr;
    }
}

void HTTPListener::setAddress(
    const std::string& host,
    uint16_t aPort
)
{
    port = aPort;
}

void HTTPListener::setAddress(
    uint32_t& ipv4,
    uint16_t aPort
)
{
    port = aPort;
}

const static char* HDR_CORS_ORIGIN = "*";
const static char* HDR_CORS_METHODS = "GET,HEAD,OPTIONS,POST,PUT,DELETE";
const static char* HDR_CORS_HEADERS = "Authorization, Access-Control-Allow-Headers, Access-Control-Allow-Origin, "
"Origin, Accept, X-Requested-With, Content-Type, Access-Control-Request-Method, Access-Control-Request-Headers";

const static char* HTTP_ERROR_404 = "Not found";
const static char* HTTP_ERROR_501 = "Not implemented";

static void addCORS(MHD_Response *response) {
    MHD_add_response_header(response, MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, HDR_CORS_ORIGIN);
    // MHD_add_response_header(response, MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_CREDENTIALS, HDR_CORS_CREDENTIALS);
    MHD_add_response_header(response, MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_METHODS, HDR_CORS_METHODS);
    MHD_add_response_header(response, MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_HEADERS, HDR_CORS_HEADERS);
}

class RequestContext {
public:
    std::string url;
    std::string postData;
};

static const char *mimeTypeByFileExtension(const std::string &filename)
{
    std::string ext = filename.substr(filename.find_last_of('.') + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext == "html" || ext == "htm")
        return CT_HTML;
    else
    if (ext == "js")
        return CT_JSON;
    else
    if (ext == "css")
        return CT_CSS;
    else
    if (ext == "png")
        return CT_PNG;
    else
    if (ext == "jpg" || ext == "jpeg")
        return CT_JPEG;
    else
    if (ext == "kml")
        return CT_KML;
    else
    if (ext == "txt")
        return CT_TEXT;
    else
    if (ext == "ttf")
        return CT_TTF;
    else
        return CT_BIN;
}

static ssize_t file_reader_callback(
    void *cls,
    uint64_t pos,
    char *buf,
    size_t max
)
{
    FILE *file = (FILE *) cls;
    (void) fseek (file, (long) pos, SEEK_SET);
    return fread (buf, 1, max, file);
}

static void free_file_reader_callback(
    void *cls
)
{
    fclose ((FILE *) cls);
}

/**
 * Translate Url to the file name
 */
static std::string buildFileName(
    const char *dirRoot,
    const char *url
)
{
    std::stringstream r;
    r << dirRoot;
    if (url) {
        r << url;
        size_t l = strlen(url);
        if (l && (url[l - 1] == '/'))
            r << DEF_HTML_INDEX_FILE_NAME;
    }
    return r.str();
}

static MHD_Result processFile(
    struct MHD_Connection *connection,
    const std::string &filename
)
{
    struct MHD_Response *response;
    MHD_Result ret;
    FILE *file;
    struct stat buf {};

    const char *localFileName = filename.c_str();
    bool gzipped = false;
    if (stat(localFileName, &buf) == 0)
        file = fopen(localFileName, "rb");
    else {
        std::string fnGzip(filename);
        fnGzip += ".gz";
        localFileName = fnGzip.c_str();
        if (stat(localFileName, &buf) == 0) {
            file = fopen(localFileName, "rb");
            gzipped = true;
        } else
            file = nullptr;
    }
    if (file == nullptr) {
        response = MHD_create_response_from_buffer(strlen(HTTP_ERROR_404), (void *) HTTP_ERROR_404, MHD_RESPMEM_PERSISTENT);
        ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
        MHD_destroy_response (response);
    } else {
        response = MHD_create_response_from_callback(buf.st_size, 32 * 1024,
            &file_reader_callback, file, &free_file_reader_callback);
        if (nullptr == response) {
            fclose (file);
            return MHD_NO;
        }
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, mimeTypeByFileExtension(filename));
        if (gzipped)
            MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_ENCODING, CE_GZIP);
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
    }
    return ret;
}

static enum MHD_Result getAllQueryString(
    void *cls,
    enum MHD_ValueKind kind,
    const char *key,
    const char *value
)
{
    auto *s = (std::string *) cls;
    if (key)
        s->append(key);
    if (value) {
        s->append("=");
        s->append(value);
    }
    return MHD_YES;
}

#ifdef ENABLE_QRCODE
typedef enum URN_TYPE {
    URN_TYPE_NONE = 0,
    URN_TYPE_STD = 1,
    URN_TYPE_PROPRIETARY = 2,
    URN_TYPE_TEXT_STD = 3,
    URN_TYPE_TEXT_PROPRIETARY = 4
} URN_TYPE;
#endif

static MHD_Result cbRequest(
	void *cls,			// HTTPListener
	struct MHD_Connection *connection,
	const char *url,
	const char *method,
	const char *version,
	const char *upload_data,
	size_t *upload_data_size,
	void **ptr
)
{
	struct MHD_Response *response;
	MHD_Result ret;

    if (!*ptr) {
		// do never respond on first call
		*ptr = new RequestContext;
		return MHD_YES;
	}

    if (strcmp(method, "OPTIONS") == 0) {
        response = MHD_create_response_from_buffer(strlen(HTTP_ERROR_501), (void *) HTTP_ERROR_501, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, CT_TEXT);
        addCORS(response);
        MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return MHD_YES;
    }

    auto *requestCtx = (RequestContext *) *ptr;
    if (*upload_data_size != 0) {
        requestCtx->postData += std::string(upload_data, *upload_data_size);
        *upload_data_size = 0;
        return MHD_YES;
    } else {
        // try read QUERY_STRING
        std::string s;
        MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, getAllQueryString, &s);
        if (!s.empty())
            requestCtx->postData += s;
    }
    requestCtx->url = url;

    int hc;
    auto *l = (HTTPListener *) cls;
#ifdef ENABLE_QRCODE
    URN_TYPE retSVG = URN_TYPE_NONE;
    if (strstr(url, "/qr")) {
        bool extended = (strstr(url, "prop") != nullptr);
        bool textOnly = (strstr(url, "text") != nullptr);
        if (textOnly) {
            if (extended)
                retSVG = URN_TYPE_TEXT_PROPRIETARY;
            else
                retSVG = URN_TYPE_TEXT_STD;
        } else {
            if (extended)
                retSVG = URN_TYPE_PROPRIETARY;
            else
                retSVG = URN_TYPE_STD;
        }
    }
#endif

    if (strcmp(method, "DELETE") == 0) {
        hc = MHD_HTTP_NOT_IMPLEMENTED;
        response = MHD_create_response_from_buffer(strlen(HTTP_ERROR_501), (void *) HTTP_ERROR_501, MHD_RESPMEM_PERSISTENT);
    } else {
        unsigned char rb[100000];
        size_t sz = 0;
        if (l->verbose > 0) {
            std::cout << method << " " << requestCtx->url;
            if (l->verbose > 1) {
                std::cout << " " << requestCtx->postData;
            }
            std::cout << std::endl;
        }
        if (l->identitySerialization) {
            sz = l->identitySerialization->query(&rb[0], sizeof(rb),
            (const unsigned char *) requestCtx->postData.c_str(), requestCtx->postData.size());
            if (sz == 0) {
                if (l->gatewaySerialization) {
                    sz = l->gatewaySerialization->query(&rb[0], sizeof(rb),
                        (const unsigned char *) requestCtx->postData.c_str(), requestCtx->postData.size());
                }
            }
            if (sz == 0) {
                if (!l->htmlRootDir.empty()) {
                    MHD_Result r = processFile(connection, buildFileName(l->htmlRootDir.c_str(), url));
                    delete requestCtx;
                    *ptr = nullptr;
                    return r;
                }
            }
        }
        if (sz == 0) {
            hc = MHD_HTTP_NOT_FOUND;
            response = MHD_create_response_from_buffer(strlen(HTTP_ERROR_404), (void *) HTTP_ERROR_404, MHD_RESPMEM_PERSISTENT);
        } else {
            hc = MHD_HTTP_OK;
#ifdef ENABLE_QRCODE
            if (retSVG > URN_TYPE_NONE) {
                std::string s = std::string((const char *) rb, sz);
                if (retSVG == URN_TYPE_STD || retSVG == URN_TYPE_TEXT_STD)
                    s = stripURNProprietary(s);
                const qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(s.c_str(), qrcodegen::QrCode::Ecc::LOW);
                if (retSVG == URN_TYPE_TEXT_STD || retSVG == URN_TYPE_TEXT_PROPRIETARY)
                    s = qrCode2Text(qr);
                else
                    s = qrCode2Svg(qr);
                response = MHD_create_response_from_buffer(s.size(), (void *) s.c_str(), MHD_RESPMEM_MUST_COPY);
            } else
#endif
                response = MHD_create_response_from_buffer(sz, (void *) &rb, MHD_RESPMEM_MUST_COPY);
        }
    }
#ifdef ENABLE_QRCODE
    if (retSVG == URN_TYPE_STD || retSVG == URN_TYPE_PROPRIETARY)
        MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, CT_SVG);
    else
        if (retSVG == URN_TYPE_TEXT_STD || retSVG == URN_TYPE_PROPRIETARY)
            MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, CT_TEXT);
        else
            MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, l->mimeType);
#else
    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, l->mimeType);
#endif
    addCORS(response);
	ret = MHD_queue_response(connection, hc, response);
	MHD_destroy_response(response);
    delete requestCtx;
    *ptr = nullptr;
	return ret;
}

int HTTPListener::run()
{
    struct MHD_Daemon *d = MHD_start_daemon(
        flags, port, nullptr, nullptr,
        &cbRequest, this,
        MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 30,  // 30s timeout
        MHD_OPTION_THREAD_POOL_SIZE, threadCount,
        // MHD_OPTION_URI_LOG_CALLBACK, &cbUriLogger, this,
        MHD_OPTION_CONNECTION_LIMIT, connectionLimit,
        MHD_OPTION_END
    );
    descriptor = (void *) d;
    return CODE_OK;
}
