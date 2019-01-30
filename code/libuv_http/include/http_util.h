#pragma once

#include <string>
#include "http_util_def.h"

namespace top {

class HttpServerUtil {
public:
    static void HandleGet(
        uv_stream_t* client, 
        const char* request_header, 
        const char* path_info, 
        const char* query_string);

    static void HandlePost(
        uv_stream_t* client, 
        const char* request_header, 
        const char* path_info, 
        const char* payload);

    static std::string HttpResponse(
        const int32_t code,
        const char* content_type,
        const char* cookie,
        const char* content);

    static std::string FormatHttpResponse(
        const char* status,
        const char* content_type,
        const char* cookie,
        const void* content,
        int32_t content_length,
        int32_t* respone_size);

    static void CloseClient(
        uv_stream_t* client);

    static void WriteUvData(
        uv_stream_t* client, 
        const void* data, 
        uint32_t len, 
        int32_t need_copy_data, 
        int32_t need_free_data);

    static void SendFile(
        uv_stream_t* client, 
        const char* content_type,
        const char* file, 
        const char* file_path);

    static std::string HandleStatusCode(
        int32_t code);

    static std::string HandleContentType(
        const char* postfix);

    static std::string HttpErrorPage(
        int32_t error_code, 
        const char* error_info);

    static int HttpPar(
        char* router_info, 
        char* path_info);

    static char* HttpHeaderParser(
        char* http_header, 
        char* key);

    static char* HttpPathParser(
        char* path, 
        int32_t i);
};

}  //  namespace top