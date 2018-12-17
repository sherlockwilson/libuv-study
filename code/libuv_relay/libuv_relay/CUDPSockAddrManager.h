#pragma once

#include <map>
#include <uv.h>
#include <mutex>


struct SockAddr
{

	SockAddr()
	{}

	SockAddr(
		const std::string& in_ip,
		const int in_port):
		ip(in_ip),
		port(in_port)
	{}

	bool operator < (const SockAddr& other) const
	{
		if (ip < other.ip)
		{
			return true;
		}
		else if (ip == other.ip)
		{
			if (port < other.port)
			{
				return true;
			}
		}
		return false;
	}

	std::string ip;
	int port;
};

class CUDPSockAddrManager
{
public:
	~CUDPSockAddrManager();

	static CUDPSockAddrManager& Instance();

	bool GetClientByServer(
		SockAddr& out_client_addr_in,
		const SockAddr& in_server_addr_in) const;

	void HashServerToClient(
		const SockAddr& in_server_addr_in,
		const SockAddr& in_client_addr_in);

private:
	CUDPSockAddrManager();
	std::map<SockAddr, SockAddr> map_server_client_;
	mutable std::mutex mutex_;
};

