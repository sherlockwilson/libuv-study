#include "uv_http_server.h"
#include "uv_http_util.h"

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
    const char* ip = "192.168.0.104";
    
    HttpServerUtil::HttpHandleRequest( request_get, request_post);
    HttpServer::Instance()->Init(static_path, ip, 8080);
    return 0;
}


