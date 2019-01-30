#pragma once

#include <uv.h>
#include <memory>
#include <string>

#if defined(WIN32)
#define snprintf _snprintf
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Userenv.lib")
#endif

#define SERVER_VERSION "0.0.1"

namespace top {

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
        const std::string& in_query,
        const std::string& in_payload,
        uv_stream_t*& in_client)
        :operate_type(in_operate_type),
        session_len(in_session_len),
        header(in_header),
        content(in_content),
        path_info(in_path_info),
        query(in_query),
        payload(in_payload),
        client(in_client)
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
    std::string query;
    uv_stream_t* client;
};

typedef std::shared_ptr<HttpSession> HttpSessionSptr;

}  //  namespace top