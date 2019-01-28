#pragma once

#include <string>
#include "http_util_def.h"

class HttpServerUtil {
public:
    static void HandleGet(
        uv_stream_t* client, 
        const char* request_header, 
        const char* path_info, 
        const char* query_stirng);

    static void HandlePost(
        uv_stream_t* client, 
        const char* request_header, 
        const char* path_info, 
        const char* payload);

    static char* HttpResponse(
        const int code,
        const char* content_type,
        const char* cookie,
        const char* content);

    static char* FormatHttpResponse(
        const char* status,
        const char* content_type,
        const char* cookie,
        const void* content,
        int content_length,
        int* respone_size);

    static void CloseClient(
        uv_stream_t* client);

    static void WriteUvData(
        uv_stream_t* client, 
        const void* data, 
        unsigned int len, 
        int need_copy_data, 
        int need_free_data);

    static void SendFile(
        uv_stream_t* client, 
        const char* content_type,
        const char* file, 
        const char* file_path);

    static std::string HandleStatusCode(
        int code);

    static std::string HandleContentType(
        const char* postfix);

    static char* HttpErrorPage(
        int error_code, 
        const char* error_info);

    static int HttpPar(
        char* router_info, 
        char* path_info);

    static char* HttpHeaderParser(
        char* http_header, 
        char* key);

    static char* HttpPathParser(
        char* path, 
        int i);
};