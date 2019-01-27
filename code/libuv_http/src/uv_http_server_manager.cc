#include "uv_http_server_manager.h"

#include "uv_http_callback.h"
#include "uv_http_server.h"

HttpServerManager* HttpServerManager::Instance() {
    static HttpServerManager ins;
    return &ins;
}

bool HttpServerManager::Init(
    const char* static_path,
    const char* ip,
    int port) {
    return HttpServer::Instance()->Init(
        static_path,
        ip,
        port);
}

