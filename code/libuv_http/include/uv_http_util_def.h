#pragma once

#include <uv.h>

#if defined(WIN32)
#define snprintf _snprintf
#endif

#define IGROPYR_VERSION "0.2.15"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Userenv.lib")

typedef char* (*igr_res)(const char* request_header, const char* path_info, const char* payload);