#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <uv.h>
#include <string>


#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Userenv.lib")

uv_loop_t *loop;
uv_udp_t send_socket;

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
}

uv_buf_t make_msg() {
	uv_buf_t buffer;
	alloc_buffer(NULL, 256, &buffer);
	memset(buffer.base, 0, buffer.len);
	std::string str_buffer("Hello Server,I'm Client!");
	memcpy(buffer.base, str_buffer.c_str(), str_buffer.length() + 1);

	return buffer;
}

void on_close_connect(uv_handle_t* handle) {

	assert(1 == uv_is_closing(handle));
	fprintf(stderr, "Close This Connect.\n");
}
void on_read_from_relay(uv_udp_t *req, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags) {
	if (nread < 0) {
		fprintf(stderr, "Read error %s\n", uv_err_name(nread));
		uv_close((uv_handle_t*)req, NULL);
		free(buf->base);
		return;
	}

	char sender[17] = { 0 };
	uv_ip4_name((const struct sockaddr_in*) addr, sender, 16);
	fprintf(stderr, "Recv from %s\n", sender);


	// ... DHCP specific code
	std::string recv_str_buf(buf->base, buf->len + 1);

	fprintf(stderr, "Offered Data %s\n", recv_str_buf.c_str());

	free(buf->base);
	uv_close((uv_handle_t*)req, on_close_connect);
}

void on_send_to_relay(uv_udp_send_t *req, int status) {
	if (status) {
		fprintf(stderr, "Send error %s\n", uv_strerror(status));
		return;
	}

	int r;

	r = uv_udp_recv_start(req->handle, alloc_buffer, on_read_from_relay);
}

int main() {
	while (true)
	{
		loop = uv_default_loop();

		uv_udp_init(loop, &send_socket);

		uv_udp_send_t send_req;
		uv_buf_t discover_msg = make_msg();

		struct sockaddr_in send_addr;
		uv_ip4_addr("192.168.1.3", 8080, &send_addr);
		uv_udp_send(&send_req, &send_socket, &discover_msg, 1, (const struct sockaddr *)&send_addr, on_send_to_relay);

		uv_run(loop, UV_RUN_DEFAULT);

		getchar();
	}
	return true;
}