#include "http_server.h"
#include "http_util.h"
#include "url_cmd_manager.h"

char* request_get(const char* head, const char * path, const char* query)
{
    const char* context = "text/html";
    const char* cookie = "";

    return HttpServerUtil::HttpResponse(200, context, cookie, query);
}

char* request_post(const char* head, const char * path, const char* payload)
{
    const char* context = "text/html";
    const char* cookie = "";

    return HttpServerUtil::HttpResponse(200, context, cookie, payload);
}

int main(void)
{
    const char* static_path = "/static/path/";
    const char* ip = "0.0.0.0";
    
    URLCmdManager::Instance()->Init();
    HttpServer::Instance()->Init(static_path, ip, 8080);
    return 0;
}


