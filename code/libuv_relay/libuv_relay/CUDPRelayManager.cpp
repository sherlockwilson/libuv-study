#include "CUDPRelayManager.h"
#include "CUDPSocketPool.h"
#include "CUVGlobalSession.h"
#include <assert.h>
#include <vector>

static uv_udp_t* ptr_udp_socket_ = nullptr;

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
	buf->base = (char*)malloc(suggested_size);
	buf->len = suggested_size;
}

void on_send_to_client(uv_udp_send_t *req, int status)
{
	if (status) {
		fprintf(stderr, "Send error %s\n", uv_strerror(status));
		return;
	}

}

void on_close_connect(uv_handle_t* handle) {

	assert(1 == uv_is_closing(handle));
	fprintf(stderr, "Close This Connect.\n");
}

void on_read_from_server(uv_udp_t *req, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags) {
	if (nread < 0) {
		fprintf(stderr, "Read error %s\n", uv_err_name(nread));
		uv_close((uv_handle_t*)req, NULL);
		free(buf->base);
		return;
	}

	char sender[17] = { 0 };
	uv_ip4_name((const struct sockaddr_in*) addr, sender, 16);
	int port = ntohs(((const struct sockaddr_in*)addr)->sin_port);
	fprintf(stderr, "Recv from %s %d\n", sender, port);


	// ... DHCP specific code
	std::string recv_str_buf(buf->base);

	fprintf(stderr, "Offered Data %s\n", recv_str_buf.c_str());

	free(buf->base);
	uv_close((uv_handle_t*)req, on_close_connect);

	uv_udp_send_t* send_req = (uv_udp_send_t*)malloc(sizeof uv_udp_send_t);

	uv_buf_t sndbuf = uv_buf_init((char*)recv_str_buf.c_str(), recv_str_buf.length());

	memset(sender, 0, 17);
	uv_ip4_name((const struct sockaddr_in*)&CUVGlobalSession::Instance().client_addr(), sender, 16);
	port = ntohs(((const struct sockaddr_in*)&CUVGlobalSession::Instance().client_addr())->sin_port);
	fprintf(stderr, "Send Data To %s %d\n", sender, port);
	
	uv_udp_send(send_req, &CUDPSocketPool::Instance().GetEnableSocket(), &sndbuf, 1, &CUVGlobalSession::Instance().client_addr(), on_send_to_client);
}

void on_send_to_server(uv_udp_send_t *req, int status) {
	if (status) {
		fprintf(stderr, "Send error %s\n", uv_strerror(status));
		return;
	}

	int r;

	r = uv_udp_recv_start(req->handle, alloc_buffer, on_read_from_server);
}

void on_read_from_client(uv_udp_t *req, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags) {
	if (nread < 0) {
		fprintf(stderr, "Read error %s\n", uv_err_name(nread));
		uv_close((uv_handle_t*)req, NULL);
		free(buf->base);
		return;
	}

	char sender[17] = { 0 };
	std::string str_sender;
	uv_ip4_name((const struct sockaddr_in*) addr, sender, 16);
	str_sender = sender;
	int port = ntohs(((const struct sockaddr_in*)addr)->sin_port);
	fprintf(stderr, "Recv from %s %d\n", str_sender.c_str(), port);
	if (!(str_sender == "192.168.1.3" && port == 9000))
	{
		CUVGlobalSession::Instance().client_addr() = *(struct sockaddr *)(addr);
	}


	// ... DHCP specific code
	std::string recv_str_buf(buf->base);

	fprintf(stderr, "Offered Data %s\n", recv_str_buf.c_str());

	free(buf->base);

	uv_udp_send_t* send_req = (uv_udp_send_t*)malloc(sizeof uv_udp_send_t);

	std::string send_str_buf(recv_str_buf);
	uv_udp_init(&CUVGlobalSession::Instance().loop(), &CUVGlobalSession::Instance().send_socket());
	uv_buf_t sndbuf = uv_buf_init((char*)send_str_buf.c_str(), send_str_buf.length());
	struct sockaddr_in send_addr;
	uv_ip4_addr("192.168.1.3", 9000, &send_addr);
	uv_udp_send(send_req, &CUVGlobalSession::Instance().send_socket(), &sndbuf, 1, (const struct sockaddr *)&send_addr, on_send_to_server);
}

CUDPRelayManager::~CUDPRelayManager()
{
}

bool CUDPRelayManager::InitManager()
{
	CUDPSocketPool::Instance().Init(100);
	for (auto& socket : CUDPSocketPool::Instance().GetAllSocket())
	{
		uv_udp_recv_start(&socket, alloc_buffer, on_read_from_client);
	}
	return true;
}

CUDPRelayManager& CUDPRelayManager::Instance()
{
	static CUDPRelayManager ins;
	return ins;
}

CUDPRelayManager::CUDPRelayManager()
{
}

bool CUDPRelayManager::Start()
{
	return uv_run(&CUVGlobalSession::Instance().loop(), UV_RUN_DEFAULT);
}
