#include "uv-listener.h"

#include <algorithm>

#include <uv.h>
#ifdef ENABLE_DEBUG
#include <iostream>
#include "lorawan-msg.h"
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <io.h>
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#define write _write
#define close _close 
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#define SOCKET int
#endif

#include "lorawan/helper/uv-mem.h"
#include "lorawan/helper/ip-helper.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-error.h"

#define DEF_KEEPALIVE_SECS 60

// 307 bytes for IPv4 up to 18, IPv6 up to 10
#define WRITE_BUFFER_SIZE 1432

static void getAddrNPort(
	uv_tcp_t *stream,
	std::string &retName,
	int &retPort
)
{
	struct sockaddr_storage name{};
	auto *n = (struct sockaddr *)&name;
	int nameSize = sizeof(name);
	if (uv_tcp_getpeername(stream, n, &nameSize)) {
        retName = "";
        retPort = 0;
		return;
	}
	char addr[INET6_ADDRSTRLEN];
	if (name.ss_family == AF_INET) {
		uv_inet_ntop(name.ss_family, &((struct sockaddr_in *)n)->sin_addr, addr, sizeof(addr));
        retPort = ntohs(((struct sockaddr_in *)n)->sin_port);
	} else {
		uv_inet_ntop(name.ss_family, &((struct sockaddr_in6 *)n)->sin6_addr, addr, sizeof(addr));
        retPort = ntohs(((struct sockaddr_in6 *)n)->sin6_port);
	}
    retName = addr;
}

/**
 * Returns IP address dotted representation, port number
 * @return true: valid IP address false: address is NULL
 */
static bool getSocketAddrNPort(
	const struct sockaddr *addr,
	std::string &retHost,
	int &retPort
)
{
	if (!addr) {
        retHost = "";
        retPort = 0;
		return false;
	}

    auto sin = (struct sockaddr_in *)addr;
	char ip[INET6_ADDRSTRLEN];
	inet_ntop(addr->sa_family, &sin->sin_addr, ip, sizeof(ip));
    retHost = std::string(ip);
    retPort = ntohs(sin->sin_port);
	return true;
}

static void onUDPRead(
	uv_udp_t *handle,
	ssize_t bytesRead,
	const uv_buf_t *buf,
	const struct sockaddr *addr,
	unsigned flags
)
{
  	if (bytesRead < 0) {
    	if (bytesRead != UV_EOF) {
#ifdef ENABLE_DEBUG
      		std::cerr << ERR_SOCKET_READ << MSG_SPACE << bytesRead << MSG_COLON_N_SPACE << uv_err_name(bytesRead) << std::endl;
#endif			
    	}
  	} else {
        if (bytesRead == 0) {

        } else {
#ifdef ENABLE_DEBUG
          std::string host;
          int port;
          getSocketAddrNPort(addr, host, port);
          std::cerr << MSG_RECEIVED
                  << hexString(buf->base, bytesRead)
                  << MSG_SPACE << MSG_OPAREN << host << ":" << port
                  << MSG_SPACE << bytesRead << MSG_SPACE << MSG_BYTES << MSG_CPAREN << std::endl;
#endif
            // 307 bytes for IPv4 up to 18, IPv6 up to 10
            unsigned char writeBuffer[WRITE_BUFFER_SIZE];
            size_t sz = ((UVListener*) handle->loop->data)->identitySerialization->query(writeBuffer,
                sizeof(writeBuffer), (const unsigned char *) buf->base, bytesRead);
            if (sz == 0) {
                sz = ((UVListener*) handle->loop->data)->gatewaySerialization->query(writeBuffer,
                    sizeof(writeBuffer), (const unsigned char *) buf->base, bytesRead);
            }
            if (sz > 0) {
                uv_buf_t wrBuf = uv_buf_init((char *) writeBuffer, (unsigned int) sz);
                auto req = (uv_udp_send_t *) malloc(sizeof(uv_udp_send_t));
                if (req) {
                    req->data = writeBuffer; // to free up if need it
                    uv_udp_send(req, handle, &wrBuf, 1, addr,
                        [](uv_udp_send_t* req, int status) {
                            if (req)
                                free(req);
                        });
                }
            }
        }
    }
	freeBuffer(buf);
}

static void onReadTCP(
	uv_stream_t *client,
	ssize_t readCount,
	const uv_buf_t *buf
)
{
	if (readCount <= 0) {
		if (readCount != UV_EOF) {
#ifdef ENABLE_DEBUG
			std::cerr << ERR_SOCKET_READ << MSG_SPACE
                << readCount << MSG_COLON_N_SPACE << uv_err_name(readCount) << std::endl;
#endif			
		}
		// client disconnected, close socket
		uv_close((uv_handle_t *)client, onCloseClient);
	} else {
#ifdef ENABLE_DEBUG
        std::string addr;
        int port;
        getAddrNPort((uv_tcp_t *)client, addr, port);
        std::cerr << MSG_RECEIVED
            << hexString(buf->base, readCount)
            << MSG_SPACE << MSG_OPAREN << "TCP " << addr << ":" << port << MSG_SPACE << readCount
            << MSG_SPACE << MSG_BYTES << MSG_CPAREN << std::endl;
#endif
        unsigned char writeBuffer[WRITE_BUFFER_SIZE];
        size_t sz = ((UVListener*) client->loop->data)->identitySerialization->query(writeBuffer,
            sizeof(writeBuffer), (const unsigned char *) buf->base, readCount);
        if (sz == 0) {
            sz = ((UVListener*) client->loop->data)->gatewaySerialization->query(writeBuffer,
                sizeof(writeBuffer), (const unsigned char *) buf->base, readCount);
        }
        if (sz > 0) {
			uv_write_t *req = allocReq();
			uv_buf_t writeBuf = uv_buf_init((char *) writeBuffer, (unsigned int) sz);
			req->data = writeBuffer; // to free up if required
			uv_write(req, client, &writeBuf, 1,
                 [](uv_write_t *req, int status) {
                     if (req)
                         free(req);
                }
            );
        }
        bool keepalive = true;
        if (!keepalive)
            uv_close((uv_handle_t *)client, onCloseClient);
	}
    freeBuffer(buf);
}

static void onConnect(
    uv_stream_t *server,
    int status
)
{
	if (status < 0) {
#ifdef ENABLE_DEBUG		
		std::cerr << ERR_SOCKET_CREATE << status << std::endl;
#endif		
		return;
	}
	uv_tcp_t *client = allocClient();
	uv_tcp_init(server->loop, client);
    uv_tcp_keepalive(client, 1, DEF_KEEPALIVE_SECS);
#ifdef ENABLE_DEBUG
    std::cerr << MSG_CONNECTED << std::endl;
#endif
    if (uv_accept(server, (uv_stream_t *)client) == 0) {
		uv_read_start((uv_stream_t *)client, allocBuffer, onReadTCP);
	} else {
		uv_close((uv_handle_t *)client, onCloseClient);
	}
}

/**
 * @see https://habr.com/ru/post/340758/
 */
UVListener::UVListener(
    IdentitySerialization *aIdentitySerialization,
    GatewaySerialization *aSerializationWrapper
)
	: StorageListener(aIdentitySerialization, aSerializationWrapper), status(CODE_OK), log(nullptr), verbose(0)
{
	uv_loop_t *loop = uv_default_loop();
    loop->data = this;
    uv = (void *) loop;
}

// http://stackoverflow.com/questions/25615340/closing-libuv-handles-correctly
void UVListener::stop()
{
	auto *uvLoop = (uv_loop_t *) uv;
    if (!uvLoop || status == ERR_CODE_STOPPED)
        return;
    status = ERR_CODE_STOPPED;
    uv_stop(uvLoop);
    int result = uv_loop_close(uvLoop);

	if (result == UV_EBUSY && uv_loop_alive(uvLoop)) {
		uv_walk(uvLoop, [](uv_handle_t* handle, void* arg) {
			if (handle->loop && !uv_is_closing(handle))
				uv_close(handle, nullptr);
			}, nullptr);
		int r;
		do {
			r = uv_run(uvLoop, UV_RUN_ONCE);
		} while (r != 0);
        uv_loop_close(uvLoop);
	}
}

void UVListener::setAddress(
    const std::string &host,
    uint16_t port
)
{
    if (isAddrStringIPv6(host.c_str()))
        uv_ip6_addr(host.c_str(), port, (sockaddr_in6*) &servaddr);
    else
        uv_ip4_addr(host.c_str(), port, (sockaddr_in*) &servaddr);
}

void UVListener::setAddress(
    uint32_t &ipv4,
    uint16_t port
)
{
    auto *a = (struct sockaddr_in *) &servaddr;
    a->sin_family = AF_INET;
    a->sin_addr.s_addr = ipv4;
    a->sin_port = htons(port);
}

int UVListener::run()
{
	auto *loop = (uv_loop_t *) uv;
	// TCP
	uv_tcp_t tcpSocket;
	uv_tcp_init(loop, &tcpSocket);
    uv_tcp_keepalive(&tcpSocket, 1, DEF_KEEPALIVE_SECS);
	uv_tcp_bind(&tcpSocket, (const struct sockaddr *)&servaddr, 0);
	int r = uv_listen((uv_stream_t *) &tcpSocket, 128, onConnect);
	if (r) {
#ifdef ENABLE_DEBUG
		std::cerr << ERR_SOCKET_LISTEN << uv_strerror(r) << std::endl;
#endif		
		status = ERR_CODE_SOCKET_LISTEN;
		return ERR_CODE_SOCKET_LISTEN;
	}

	// UDP
	uv_udp_t udpSocket;
	uv_udp_init(loop, &udpSocket);
	r = uv_udp_bind(&udpSocket, (const struct sockaddr *)&servaddr, UV_UDP_REUSEADDR);
    if (r) {
#ifdef ENABLE_DEBUG
        std::cerr << ERR_SOCKET_BIND << uv_strerror(r) << std::endl;
#endif
        status = ERR_CODE_SOCKET_BIND;
        return ERR_CODE_SOCKET_BIND;
    }
	uv_udp_set_broadcast(&udpSocket, 1);
	r = uv_udp_recv_start(&udpSocket, allocBuffer, onUDPRead);
	if (r) {
#ifdef ENABLE_DEBUG
		std::cerr << ERR_SOCKET_LISTEN << uv_strerror(r) << std::endl;
#endif
		status = ERR_CODE_SOCKET_LISTEN;
		return ERR_CODE_SOCKET_LISTEN;
	}
	status = CODE_OK;
	uv_run(loop, UV_RUN_DEFAULT);
    return status;
}

/**
 * 	Call stop() before destroy
 */
UVListener::~UVListener()
{
}

void UVListener::setLog(
    int aVerbose,
    Log *aLog
)
{
    verbose = aVerbose;
    log = aLog;
}
