#pragma once

#include <uv.h>

#include <string>

#include "http_util_def.h"

namespace top {

class HttpServerManager {
public:
    static HttpServerManager* Instance();
    bool Init(
        const char* static_path, 
        const char* ip, 
        int port);
private:
    HttpServerManager() {
    }
    ~HttpServerManager() {
    }
};

}  //  namespace top