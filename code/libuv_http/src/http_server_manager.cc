#include "http_server_manager.h"

#include "http_callback.h"
#include "http_server.h"
#include "cmd_manager.h"
#include "message_loop.h"

namespace top {

HttpServerManager* HttpServerManager::Instance() {
    static HttpServerManager ins;
    return &ins;
}

bool HttpServerManager::Init(
    const char* static_path,
    const char* ip,
    int port) {
    MessageLoopForGet::Instance()->Init();
    CmdManager::Instance()->Init();
    return HttpServer::Instance()->Init(
        static_path,
        ip,
        port);
}

}  //  namespace top