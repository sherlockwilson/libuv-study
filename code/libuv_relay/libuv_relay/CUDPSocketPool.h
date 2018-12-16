#pragma once

#include <uv.h>
#include <vector>

class CUDPSocketPool
{
public:
	~CUDPSocketPool();

	bool Init(int in_socket_num);

	static CUDPSocketPool& Instance();

	uv_udp_t& GetEnableSocket();

	std::vector<uv_udp_t>& GetAllSocket();

private:
	CUDPSocketPool();
	std::vector<uv_udp_t> vt_udp_socket_;
};

