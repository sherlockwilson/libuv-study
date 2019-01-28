#pragma once

#include <uv.h>

#include <string>

#include "http_util_def.h"

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
private:
    HttpServer() {
    }
    ~HttpServer() {
    }
    uv_tcp_t server_;
    igr_res res_get_;
    igr_res res_post_;
    std::string url_;
};