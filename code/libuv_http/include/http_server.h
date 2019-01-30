#pragma once

#include <uv.h>

#include <string>

#include "http_util_def.h"

namespace top {

class HttpServer {
public:
    static HttpServer* Instance();
    bool Init(
        const char* static_path, 
        const char* ip, 
        int port);
    uv_tcp_t* server() {
        return &server_;
    }
    igr_res& res_get() {
        return res_get_;
    }
    igr_res& res_post() {
        return res_post_;
    }
    std::string& url() {
        return url_;
    }
    std::string& ip() {
        return ip_;
    }
    int32_t& port() {
        return port_;
    }
private:
    HttpServer() {
    }
    ~HttpServer() {
    }
    uv_tcp_t server_;
    igr_res res_get_;
    igr_res res_post_;
    std::string url_;
    std::string ip_;
    int32_t port_;
};

}  //  namespace top