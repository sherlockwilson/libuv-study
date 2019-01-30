#include "http_server.h"

#include "http_callback.h"

namespace top {

HttpServer* HttpServer::Instance() {
    static HttpServer ins;
    return &ins;
}

bool HttpServer::Init(
    const char* static_path,
    const char* ip,
    int port) {
    struct sockaddr_in addr;
    uv_ip4_addr(ip, port, &addr);
    url_ = static_path;
    uv_tcp_init(uv_default_loop(), &server_);
    uv_tcp_bind(&server_, (const struct sockaddr*) &addr, 0);
    uv_listen((uv_stream_t*)&server_, 8, HttpServerCallBack::OnConnection);

    return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

}  //  namespace top