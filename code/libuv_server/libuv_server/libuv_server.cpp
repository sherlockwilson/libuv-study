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
uv_udp_t recv_socket;

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
}

void on_send_to_relay(uv_udp_send_t *req, int status)
{
	if (status) {
		fprintf(stderr, "Send error %s\n", uv_strerror(status));
		return;
	}

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
	std::string recv_str_buf(buf->base);

	fprintf(stderr, "Offered Data %s\n", recv_str_buf.c_str());

	free(buf->base);

	uv_udp_send_t* send_req = (uv_udp_send_t*)malloc(sizeof uv_udp_send_t);
	
	std::string send_str_buf("Hello Client,I'm Server");

	uv_buf_t sndbuf = uv_buf_init((char*)send_str_buf.c_str(), send_str_buf.length() + 1);
	 uv_udp_send(send_req, req, &sndbuf, 1, addr, on_send_to_relay);
}



int main() {
	loop = uv_default_loop();

	uv_udp_init(loop, &recv_socket);
	struct sockaddr_in recv_addr;
	uv_ip4_addr("192.168.1.3", 9000, &recv_addr);
	uv_udp_bind(&recv_socket, (const struct sockaddr *)&recv_addr, UV_UDP_REUSEADDR);
	uv_udp_recv_start(&recv_socket, alloc_buffer, on_read_from_relay);

	int ret = uv_run(loop, UV_RUN_DEFAULT);

	getchar();
	return ret;
}