#include"CUDPRelayManager.h"


#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Userenv.lib")


int main() {
	CUDPRelayManager::Instance().InitManager();
	CUDPRelayManager::Instance().Start();

	getchar();
	return true;
}