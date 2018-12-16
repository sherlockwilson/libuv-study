#include "CUDPSocketPool.h"
#include "CUVGlobalSession.h"
#include <time.h>


CUDPSocketPool::~CUDPSocketPool()
{
}

bool CUDPSocketPool::Init(int in_socket_num)
{

	srand(NULL);

	for (uint32_t i = 0; i < in_socket_num; ++i)
	{
		uv_udp_t recv_socket;
		vt_udp_socket_.push_back(recv_socket);
	}

	CUVGlobalSession::Instance().loop() = *uv_default_loop();

	for (auto& udp_socket : vt_udp_socket_)
	{
		uv_udp_init(&CUVGlobalSession::Instance().loop(), &udp_socket);

		uv_ip4_addr("192.168.1.3", 8080, &CUVGlobalSession::Instance().server_addr());
		uv_udp_bind(&udp_socket, (const struct sockaddr *)&CUVGlobalSession::Instance().server_addr(), UV_UDP_REUSEADDR);
	}

	return true;
}


CUDPSocketPool& CUDPSocketPool::Instance()
{
	static CUDPSocketPool ins;
	return ins;
}

uv_udp_t& CUDPSocketPool::GetEnableSocket()
{
	int max_size = vt_udp_socket_.size();
	int index = rand() % max_size;
	return vt_udp_socket_[index];
}

std::vector<uv_udp_t>& CUDPSocketPool::GetAllSocket()
{
	return vt_udp_socket_;
}

CUDPSocketPool::CUDPSocketPool()
{
}

