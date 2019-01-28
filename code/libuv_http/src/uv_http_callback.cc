#include "uv_http_callback.h"

#include <assert.h>

#include <sstream>
#include <thread>

#include "membuf.h"
#include "uv_http_server.h"
#include "uv_http_util.h"
#include "uv_http_client_manager.h"


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
    if (nread > 0) {

        bool ret = HttpClientSessionManager::Instance()->HaveKey(client);

        if (!ret) {
            char* separator;
            char* payload;
            char* query_stirng;
            char* end;
            membuf_t* membuf = (membuf_t*)client->data;
            assert(membuf);

            Membuf::AppendData(membuf, buf->base, nread);
            separator = const_cast<char*>(strstr((const char*)membuf->data, "\r\n\r\n"));

            std::string data((const char*)membuf->data, membuf->size);
            if (separator != NULL) {
                std::string request_header = (const char*)membuf->data;
                *separator = '\0';
                payload = separator + 4;

                if (request_header[0] == 'G'
                    && request_header[1] == 'E'
                    && request_header[2] == 'T') {
                    const char* path_info = request_header.c_str() + 3;

                    while (isspace(*path_info)) {
                        path_info++;
                    }

                    end = const_cast<char*>(strchr(path_info, ' '));

                    if (end) {
                        *end = '\0';
                        request_header = end + 1;
                    }

                    query_stirng = const_cast<char*>(strchr(path_info, '?'));

                    if (query_stirng) {
                        *query_stirng = '\0';
                        query_stirng++;
                    }

                    HttpServerUtil::HandleGet(
                        client,
                        request_header.c_str(),
                        path_info,
                        query_stirng);
                }
                else if (
                    request_header[0] == 'P'
                    && request_header[1] == 'O'
                    && request_header[2] == 'S'
                    && request_header[3] == 'T') {
                    const char* path_info = request_header.c_str() + 4;

                    while (isspace(*path_info)) path_info++;
                    end = const_cast<char*>(strchr(path_info, ' '));

                    if (end) {
                        *end = '\0';
                        request_header = end + 1;
                    }

                    printf("request_header:%s\n",
                        request_header.c_str());

                    std::string key("Content-Length: ");
                    int32_t it_find = request_header.find(key.c_str());
                    if (std::string::npos != it_find) {
                        int32_t begin_of_num = it_find + key.size();
                        int32_t end_of_num = request_header.find("\r\n", begin_of_num);
                        std::string num = request_header.substr(
                            begin_of_num,
                            end_of_num - begin_of_num);
                        std::stringstream sin(num);
                        int32_t content_length;
                        if (!(sin >> content_length)) {
                            return;
                        }
                        printf("%s%d\n",
                            key.c_str(),
                            content_length);
                        auto sptr_http_post = std::make_shared<HttpSession>();
                        sptr_http_post->local_content += request_header;
                        sptr_http_post->content_len = content_length;
                        HttpClientSessionManager::Instance()->AddData(client, sptr_http_post);
                    }

                    HttpServerUtil::HandlePost(
                        client,
                        request_header.c_str(),
                        path_info,
                        payload);

                }
                else {
                    HttpServerUtil::CloseClient(client);
                }
            }
        } else {
            /////////////


            HttpClientSessionManager::Instance()->DeleteKey(client);
        }
    } else if (nread == -1) {
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

