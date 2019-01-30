#include "http_callback.h"

#include <assert.h>
#include <string.h>

#include <sstream>
#include <thread>

#include "membuf.h"
#include "http_server.h"
#include "http_util.h"

namespace top {

void HttpServerCallBack::OnUvAlloc(
    uv_handle_t* handle, 
    size_t suggested_size, 
    uv_buf_t* buf) {
    *buf = uv_buf_init(
        reinterpret_cast<char*>(malloc(suggested_size)), 
        suggested_size);
}

void HttpServerCallBack::OnUvRead(
    uv_stream_t* client, 
    ssize_t nread, 
    const uv_buf_t* buf) {
    printf("nread:%d\n", nread);
    if (nread > 0) {
        char* separator;
        char* payload;
        char* query_string;
        char* end;
        membuf_t* membuf = (membuf_t*)client->data;
        assert(membuf);

        Membuf::AppendData(membuf, buf->base, nread);
        separator = const_cast<char*>(strstr((const char*)membuf->data, "\r\n\r\n"));

        std::string data((const char*)membuf->data, membuf->size);
        if (separator != NULL) {
            char* request_header = (char*)membuf->data;
            *separator = '\0';
            payload = separator + 4;

            if (request_header[0] == 'G'
                && request_header[1] == 'E'
                && request_header[2] == 'T') {
                const char* path_info = request_header + 3;

                while (isspace(*path_info)) {
                    path_info++;
                }

                end = const_cast<char*>(strchr(path_info, ' '));

                if (end) {
                    *end = '\0';
                    request_header = end + 1;
                }

                query_string = const_cast<char*>(strchr(path_info, '?'));

                if (query_string) {
                    *query_string = '\0';
                    query_string++;
                }
                printf("path_info:%s\n",path_info);
                printf("query_string:%s\n",query_string);
                HttpServerUtil::HandleGet(
                    client,
                    request_header,
                    path_info,
                    query_string);
            }
            else if (
                request_header[0] == 'P'
                && request_header[1] == 'O'
                && request_header[2] == 'S'
                && request_header[3] == 'T') {
                const char* path_info = request_header + 4;

                while (isspace(*path_info)) path_info++;
                end = const_cast<char*>(strchr(path_info, ' '));

                if (end) {
                    *end = '\0';
                    request_header = end + 1;
                }

                printf("request_header:%s\n",
                    request_header);
                /*
                std::string key("Content-Length: ");
                char* it_find = std::find(request_header, request_header + strlen(request_header), key.c_str());
                if (NULL != it_find) {
                    char* it_find1 = std::find(it_find + key.size(), request_header + strlen(request_header), "\r\n");
                    char number[256] = {0};
                    strncpy(number, it_find, it_find1 - it_find1);
                    int32_t content_length = atoi(number);
                    printf("%s%d\n",
                        key.c_str(),
                        content_length);
                }*/

                HttpServerUtil::HandlePost(
                    client,
                    request_header,
                    path_info,
                    payload);

            }
            else {
                HttpServerUtil::CloseClient(client);
            }
        }
    }
    else if (nread == -1) {
        HttpServerUtil::CloseClient(client);
    }

    if (buf->base) {
        free(buf->base);
    }
}

void HttpServerCallBack::OnConnection(
    uv_stream_t* server, 
    int status) {
    assert(server == (uv_stream_t*)HttpServer::Instance()->server());
    if (status == 0) {
        uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
        client->data = malloc(sizeof(membuf_t));
        Membuf::Init((membuf_t*)client->data, 8096);
        uv_tcp_init(uv_default_loop(), client);
        uv_accept((uv_stream_t*)HttpServer::Instance()->server(), (uv_stream_t*)client);
        uv_read_start((uv_stream_t*)client, OnUvAlloc, OnUvRead);
    }
}

void HttpServerCallBack::AfterUvCloseClient(
    uv_handle_t* client) {
    Membuf::Uninit((membuf_t*)client->data);
    free(client->data);
    free(client);
}

void HttpServerCallBack::AfterUvWrite(
    uv_write_t* w, 
    int status)
{
    uv_close((uv_handle_t*)w->handle, AfterUvCloseClient);
    free(w);
}

} //  namespace top