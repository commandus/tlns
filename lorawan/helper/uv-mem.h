#include <uv.h>

void allocBuffer(
	uv_handle_t *handle,
	size_t suggested_size,
	uv_buf_t *buf
);

void freeBuffer(
	const uv_buf_t *buf
);

uv_write_t *allocReq();

void freeReqData(uv_write_t *req);

void freeReq(
	uv_write_t *req
);

uv_tcp_t *allocClient();

void freeClient(void *value);

void freeUDP(uv_udp_send_t *value);

void onCloseClient(uv_handle_t *handle);
