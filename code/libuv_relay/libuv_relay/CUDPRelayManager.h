#pragma once

#include <uv.h>

class CUDPRelayManager
{
public:

	static CUDPRelayManager& Instance();

	~CUDPRelayManager();

	bool InitManager();

	bool Start();

private:
	CUDPRelayManager();
};

