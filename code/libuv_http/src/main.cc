#include "http_server_manager.h"

int main(void) {
    const char* static_path = "/static/path/";
    const char* ip = "0.0.0.0";
    top::HttpServerManager::Instance()->Init(static_path, ip, 0);
    return 0;
}


