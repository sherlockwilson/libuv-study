#pragma once

#include <uv.h>

#include <string>

#if defined(WIN32)
#define snprintf _snprintf
#endif

#define SERVER_VERSION "0.0.1"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Userenv.lib")

typedef char* (*igr_res)(const char* request_header, const char* path_info, const char* payload);

enum OperateType {
    POST = 1,
    GET = 2
};

struct HttpSession {
    HttpSession() {}
    HttpSession(
        const OperateType in_operate_type,
        const int32_t in_session_len,
        const std::string& in_header,
        const std::string& in_content,
        const std::string& in_path_info,
        const std::string& in_payload)
        :operate_type(in_operate_type),
        session_len(in_session_len),
        header(in_header),
        content(in_content),
        path_info(in_path_info),
        payload(in_payload)
    {}
    ~HttpSession() {}

    bool IsExpire() const {
        return header.length() + 
            content.length() >= 
            session_len;
    }
    OperateType operate_type;
    int32_t session_len;
    std::string header;
    std::string content;
    std::string path_info;
    std::string payload;
};

typedef std::shared_ptr<HttpSession> HttpSessionSptr;