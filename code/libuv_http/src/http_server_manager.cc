#include "http_server_manager.h"

#include "http_callback.h"
#include "http_server.h"
#include "url_cmd_manager.h"
#include "message_loop.h"

HttpServerManager* HttpServerManager::Instance() {
    static HttpServerManager ins;
    return &ins;
}

bool HttpServerManager::Init(
    const char* static_path,
    const char* ip,
    int port) {
    URLCmdManager::Instance()->Init();
    MessageLoopForGet::Instance()->Init();
    return HttpServer::Instance()->Init(
        static_path,
        ip,
        port);
}

