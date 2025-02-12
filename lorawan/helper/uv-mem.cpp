#include <cstdlib>
#include "uv-mem.h"

void allocBuffer(
	uv_handle_t *handle,
	size_t suggested_size,
	uv_buf_t *buf
)
{
	buf->base = (char *) malloc(suggested_size);
	buf->len = (unsigned long) suggested_size;
}

void freeBuffer(
	const uv_buf_t *buf
)
{
	if (buf) {
		if (buf->base) {
			free(buf->base);
		}
	}
}

uv_write_t *allocReq()
{
	return (uv_write_t *)malloc(sizeof(uv_write_t));
}

void freeReqData(uv_write_t *req)
{
	if (req) {
		if (req->data) {
			free(req->data);
			req->data = nullptr;
		}
	}
}

void freeReq(
	uv_write_t *req
)
{
	if (req) {
		freeReqData(req);
		free(req);
	}
}

uv_tcp_t *allocClient()
{
	return (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
}

void freeClient(void *value)
{
	if (value) {
		free(value);
	}
}

void freeUDP(uv_udp_send_t *value)
{
	if (value) {
		if (value->data)
			free(value->data);
        free(value);
	}
}

void onCloseClient(uv_handle_t *handle)
{
	freeClient(handle);
}
