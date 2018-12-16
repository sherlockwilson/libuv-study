#pragma once

#include <uv.h>

class CUVGlobalSession
{
public:

	static CUVGlobalSession& Instance();

	~CUVGlobalSession();

	uv_loop_t& loop();

	uv_udp_t& send_socket();

	uv_udp_t& recv_socket();

	struct sockaddr& client_addr();

	struct sockaddr_in& server_addr();
private:
	CUVGlobalSession();
	uv_loop_t loop_;
	uv_udp_t send_socket_;
	uv_udp_t recv_socket_;
	struct sockaddr client_addr_;
	struct sockaddr_in server_addr_;
};

