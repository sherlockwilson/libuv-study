#include "CUDPSockAddrManager.h"
#include <memory>


CUDPSockAddrManager::CUDPSockAddrManager()
{
}


CUDPSockAddrManager::~CUDPSockAddrManager()
{
}


CUDPSockAddrManager& CUDPSockAddrManager::Instance()
{
	static CUDPSockAddrManager ins;
	return ins;
}

bool CUDPSockAddrManager::GetClientByServer(
	SockAddr& out_client_addr_in,
	const SockAddr& in_server_addr_in) const
{
	std::lock_guard<std::mutex> lock(mutex_);
	auto it_find = map_server_client_.find(in_server_addr_in);
	if (it_find == map_server_client_.end())
	{
		return false;
	}
	out_client_addr_in = it_find->second;
	return true;
}


void CUDPSockAddrManager::HashServerToClient(
	const SockAddr& in_server_addr_in,
	const SockAddr& in_client_addr_in)
{
	std::lock_guard<std::mutex> lock(mutex_);
	map_server_client_[in_server_addr_in] = in_client_addr_in;
}