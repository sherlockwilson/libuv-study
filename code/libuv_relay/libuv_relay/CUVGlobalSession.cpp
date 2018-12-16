#include "CUVGlobalSession.h"



CUVGlobalSession::CUVGlobalSession()
{
}


CUVGlobalSession::~CUVGlobalSession()
{
}


CUVGlobalSession& CUVGlobalSession::Instance()
{
	static CUVGlobalSession ins;
	return ins;
}

uv_loop_t& CUVGlobalSession::loop()
{
	return loop_;
}

uv_udp_t& CUVGlobalSession::send_socket()
{
	return send_socket_;
}

uv_udp_t& CUVGlobalSession::recv_socket()
{
	return recv_socket_;
}

struct sockaddr& CUVGlobalSession::client_addr()
{
	return client_addr_;
}

struct sockaddr_in& CUVGlobalSession::server_addr()
{
	return server_addr_;
}
