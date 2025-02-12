#include "uv-client.h"

#include <uv.h>

#include "lorawan/helper/ip-helper.h"
#include "lorawan/storage/serialization/gateway-binary-serialization.h"

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <io.h>
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#define write _write
#define close _close 
#else
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

#define SOCKET int
#endif

#include "lorawan/helper/uv-mem.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-conv.h"
#include "lorawan/storage/serialization/identity-binary-serialization.h"

#ifdef ENABLE_DEBUG
#include <iostream>
#include <cstring>
#include "lorawan/lorawan-msg.h"
#endif

#define DEF_KEEPALIVE_SECS 60

static void parseResponse(
    UvClient *client,
    const unsigned char *buf,
    ssize_t nRead
) {
    if (!client)
        return;
    if (isIdentityTag(buf, nRead)) {
        enum IdentityQueryTag tag = validateIdentityQuery(buf, nRead);
        switch (tag) {
            case QUERY_IDENTITY_EUI:   // request gateway identifier(with address) by network address.
            case QUERY_IDENTITY_ADDR:   // request gateway address (with identifier) by identifier.
            {
                IdentityGetResponse gr(buf, nRead);
                gr.ntoh();
                client->onResponse->onIdentityGet(client, &gr);
            }
                break;
            case QUERY_IDENTITY_LIST:   // List entries
            {
                IdentityListResponse gr(buf, nRead);
                gr.response = NTOH4(gr.response);
                gr.ntoh();
                client->onResponse->onIdentityList(client, &gr);
            }
                break;
            default: {
                IdentityOperationResponse gr(buf, nRead);
                gr.ntoh();
                client->onResponse->onIdentityOperation(client, &gr);
            }
                break;
        }

    } else {
        enum GatewayQueryTag tag = validateGatewayQuery(buf, nRead);
        switch (tag) {
            case QUERY_GATEWAY_ADDR:   // request gateway identifier(with address) by network address.
            case QUERY_GATEWAY_ID:   // request gateway address (with identifier) by identifier.
            {
                GatewayGetResponse gr(buf, nRead);
                gr.ntoh();
                client->onResponse->onGatewayGet(client, &gr);
            }
                break;
            case QUERY_GATEWAY_LIST:   // List entries
            {
                GatewayListResponse gr(buf, nRead);
                gr.response = NTOH4(gr.response);
                gr.ntoh();
                client->onResponse->onGatewayList(client, &gr);
            }
                break;
            default: {
                GatewayOperationResponse gr(buf, nRead);
                gr.ntoh();
                client->onResponse->onGatewayOperation(client, &gr);
            }
                break;
        }
    }
}

static void onTCPRead(
	uv_stream_t* strm,
	ssize_t nRead,
	const uv_buf_t* buf
) {
    auto client = (UvClient*) strm->data;
    if (!client)
        return;
    if (nRead < 0) {
        if (nRead == UV__EOF) {
            client->tcpConnected = false;
            client->onResponse->onError(client, ERR_CODE_SOCKET_READ, (int) nRead);
            return;
        } else {
#ifdef ENABLE_DEBUG
            std::cerr << ERR_SOCKET_READ << MSG_SPACE << nRead << MSG_COLON_N_SPACE << uv_err_name(nRead) << std::endl;
#endif
        }
    } else {
#ifdef ENABLE_DEBUG
        std::cerr << MSG_READ_BYTES << hexString(buf->base, nRead) << MSG_OPAREN << nRead << MSG_BYTES << MSG_CPAREN << std::endl;
#endif
        parseResponse(client, (const unsigned char *) buf->base, nRead);
    }
    freeBuffer(buf);
}

// https://gist.github.com/Jxck/4305806
static void onWriteEnd(
	uv_write_t* req,
	int status
) {
	if (status < 0) {
#ifdef ENABLE_DEBUG		
		std::cerr << ERR_SOCKET_WRITE << status << std::endl;
#endif
        freeReqData(req);
        if (req)
            free(req);
    }
}

static void onUDPread(
    uv_udp_t *handle,
    ssize_t bytesRead,
    const uv_buf_t *buf,
    const struct sockaddr *addr,
    unsigned flags
) {
    if (bytesRead < 0) {
        if (bytesRead != UV__EOF) {
#ifdef ENABLE_DEBUG
            std::cerr << ERR_SOCKET_READ << MSG_SPACE << bytesRead << MSG_COLON_N_SPACE << uv_err_name(bytesRead) << std::endl;
#endif
        }
        return;
    }
    auto client = (UvClient*) handle->data;
    if (bytesRead == 0) {
        return;
    } else {
#ifdef ENABLE_DEBUG
        std::cerr << MSG_READ_BYTES << hexString(buf->base, bytesRead) << MSG_OPAREN << bytesRead << MSG_CPAREN << std::endl;
#endif
    }
    parseResponse(client, (const unsigned char *) buf->base, bytesRead);
    freeBuffer(buf);
}

static void onClientUDPSent(
    uv_udp_send_t *req,
    int status
) {
    if (!req)
        return;
    auto *client = (UvClient *) req->data;
    if (status) {
#ifdef ENABLE_DEBUG
        std::cerr << ERR_SOCKET_WRITE << status << MSG_COLON_N_SPACE << uv_strerror(status) << std::endl;
#endif
        client->onResponse->onError(client, ERR_CODE_SOCKET_WRITE, status);
        free(req);
        return;
    }
    free(req);
}

static int sendTcp(
    UvClient* client,
    uv_stream_t* tcp
)
{
    uv_buf_t buf = uv_buf_init((char *) client->dataBuf, (unsigned int) client->dataSize);
    auto *write_req = new uv_write_t;
    write_req->data = client;
    int buf_count = 1;
    int r = uv_write(write_req, tcp, &buf, buf_count, onWriteEnd);
    if (r < 0) {
#ifdef ENABLE_DEBUG
        std::cerr << ERR_SOCKET_WRITE << r << MSG_COLON_N_SPACE << uv_strerror(r) << std::endl;
#endif
        client->onResponse->onError(client, ERR_CODE_SOCKET_WRITE, r);
    }
    return r;
}

static void onConnect(
	uv_connect_t* req,
	int status
)
{
	if (status < 0) {
#ifdef ENABLE_DEBUG		
		std::cerr << ERR_SOCKET_CONNECT << status << MSG_COLON_N_SPACE << uv_strerror(status) << std::endl;
#endif
        if (status != -125)
		    return;
	}
    auto client = (UvClient*) req->data;
    client->tcpConnected = true;
    uv_read_start(req->handle, allocBuffer, onTCPRead);
    sendTcp((UvClient*) req->data, req->handle);
#ifdef ENABLE_DEBUG
    std::cerr << MSG_CONNECTED << std::endl;
#endif
}

void UvClient::init()
{
    loop = uv_default_loop();
    if (useTcp) {
        // TCP
        tcpConnected = false;
        uv_tcp_init(loop, &tcpSocket);
        uv_tcp_keepalive(&tcpSocket, 1, DEF_KEEPALIVE_SECS);
        tcpSocket.data = this;
    } else {
        // UDP
        uv_udp_init(loop, &udpSocket);
        udpSocket.data = this;
        int r = uv_udp_recv_start(&udpSocket, allocBuffer, onUDPread);
        if (r < 0) {
#ifdef ENABLE_DEBUG
            std::cerr << ERR_SOCKET_READ << r << MSG_COLON_N_SPACE << uv_strerror(r) << std::endl;
            return;
#endif
        }
    }
}

void UvClient::stop()
{
    if (useTcp) {
        // TCP
        tcpConnected = false;
    }
    if (!loop)
        return;
    uv_stop(loop);
}

void UvClient::finish()
{
    int result = uv_loop_close(loop);
    if (result == UV_EBUSY && uv_loop_alive(loop)) {
        uv_walk(loop, [](uv_handle_t* handle, void* arg) {
            if (handle->loop && !uv_is_closing(handle))
                uv_close(handle, nullptr);
        }, nullptr);
        int r;
        do {
            r = uv_run(loop, UV_RUN_ONCE);
        } while (r != 0);
        uv_loop_close(loop);
    }
}

/**
 */
UvClient::UvClient(
        bool aUseTcp,
        const std::string &aHost,
        uint16_t aPort,
        ResponseClient *aOnResponse
)
	: QueryClient(aOnResponse), useTcp(aUseTcp), status(0), tcpConnected(false), query(nullptr)
{
    int r;
    if (isAddrStringIPv6(aHost.c_str()))
        r = uv_ip6_addr(aHost.c_str(), aPort, (sockaddr_in6*) &serverAddress);
    else
        r = uv_ip4_addr(aHost.c_str(), aPort, (sockaddr_in*) &serverAddress);
    if (r)
        status = ERR_CODE_ADDR_OUT_OF_RANGE;
    init();
}

void UvClient::initiateQuery()
{
    query->ntoh();
    size_t sz = query->serialize((unsigned char *) sendBuffer);
#ifdef ENABLE_DEBUG
    std::cerr << MSG_QUERY << MSG_SPACE << sz << MSG_SPACE << MSG_BYTES;
    if (sz > 0)
        std::cerr << MSG_COLON_N_SPACE << hexString(sendBuffer, sz);
    std::cerr << std::endl;
#endif
	int r;
    dataBuf = sendBuffer;
    dataSize = sz;
	if (useTcp) {
        // TCP
        if (!tcpConnected) {
            connectReq.data = this;
            r = uv_tcp_connect(&connectReq, &tcpSocket, (const sockaddr *) &serverAddress, onConnect);
        } else {
            r = sendTcp(this, connectReq.handle);
        }
	} else {
        // UDP
        auto *sendReq = (uv_udp_send_t *) malloc(sizeof(uv_udp_send_t));
        uv_buf_t uvBuf = uv_buf_init((char *) sendBuffer, (unsigned int) sz);
        if (sendReq) {
            sendReq->data = this;
            r = uv_udp_send(sendReq, &udpSocket, &uvBuf, 1, (const struct sockaddr*)&serverAddress, onClientUDPSent);
        }
        else
            r = ERR_CODE_INSUFFICIENT_MEMORY;
    }
	if (r) {
#ifdef ENABLE_DEBUG
		std::cerr << ERR_MESSAGE << r << MSG_COLON_N_SPACE << uv_strerror(r) << std::endl;
#endif
		status = ERR_CODE_SOCKET_CREATE;
        onResponse->onError(this, ERR_CODE_SOCKET_READ, r);
		return;
	}
	status = CODE_OK;
}

UvClient::~UvClient()
{
    stop();
}

ServiceMessage* UvClient::request(
    ServiceMessage* value
)
{
    ServiceMessage* r = query;
    query = value;
    initiateQuery();
    return r;
}

void UvClient::start() {
    uv_run(loop, UV_RUN_DEFAULT);
    finish();
}
