#include "uv_http_util.h"
#include "uv_http_server.h"
#include "uv_http_callback.h"

#include <assert.h>

void HttpServerUtil::HandleGet(
    uv_stream_t* client, 
    const char* request_header, 
    const char* path_info, 
    const char* query_stirng) {
    char* postfix = strrchr(const_cast<char*>(path_info), '.');
    const char* url = HttpServer::Instance()->url().c_str();
    if (postfix) {
        postfix++;
        if (url) {
            char file[1024];
            snprintf(file, sizeof(file), "%s%s", url, path_info);
            SendFile(client, HandleContentType(postfix).c_str(), file, path_info);
            return;
        } else {
            char* respone = HttpErrorPage(403, path_info);
            WriteUvData(client, respone, -1, 0, 1);
            return;
        }
    } else {
        char* respone = HttpServer::Instance()->res_get()(request_header, path_info, query_stirng);
        if (*respone == ' ') {
            if (url) {
                char* file_path = respone;
                file_path++;
                postfix = strrchr(file_path, '.');
                postfix++;
                char file[1024];
                snprintf(file, sizeof(file), "%s%s", url, file_path);
                SendFile(client, HandleContentType(postfix).c_str(), file, path_info);
                return;
            } else {
                char* respone = HttpErrorPage(403, path_info);
                WriteUvData(client, respone, -1, 0, 1);
                return;
            }
        } else {
            WriteUvData(client, respone, -1, 0, 1);
            return;
        }
    }
}

void HttpServerUtil::HandlePost(
    uv_stream_t* client, 
    const char* request_header, 
    const char* path_info, 
    const char* payload) {
    char* respone = HttpServer::Instance()->res_post()(request_header, path_info, payload);
    const char* url = HttpServer::Instance()->url().c_str();

    if (*respone == ' ') {
        if (url) {
            char* file_path = respone;
            file_path++;
            char* postfix = strrchr(file_path, '.');
            postfix++;
            char file[1024];
            snprintf(file, sizeof(file), "%s%s", url, file_path);
            SendFile(client, HandleContentType(postfix).c_str(), file, path_info);
            return;
        } else {
            char* respone = HttpErrorPage(403, path_info);
            WriteUvData(client, respone, -1, 0, 1);
            return;
        }
    } else {
        WriteUvData(client, respone, -1, 0, 1);
        return;
    }
}

void HttpServerUtil::CloseClient(
    uv_stream_t* client) {
    uv_close((uv_handle_t*)client, HttpServerCallBack::AfterUvCloseClient);
}

void HttpServerUtil::WriteUvData(
    uv_stream_t* client,
    const void* data,
    unsigned int len,
    int need_copy_data,
    int need_free_data) {
    uv_buf_t buf;
    uv_write_t* w;
    void* newdata = (void*)data;

    if (data == NULL || len == 0) {
        return;
    }
    if (len == (unsigned int)-1) {
        len = strlen((char*)data);
    }
    if (need_copy_data) {
        newdata = malloc(len);
        memcpy(newdata, data, len);
    }

    buf = uv_buf_init((char*)newdata, len);
    w = (uv_write_t*)malloc(sizeof(uv_write_t));
    w->data = (need_copy_data || need_free_data) ? newdata : NULL;
    uv_write(w, client, &buf, 1, HttpServerCallBack::AfterUvWrite);
}

void HttpServerUtil::SendFile(
    uv_stream_t* client, 
    const char* content_type, 
    const char* file, 
    const char* file_path) {
    int file_size;
    int read_bytes;
    int respone_size;
    char* respone;
    unsigned char* file_data;


    FILE* fp = fopen(file, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        file_data = (unsigned char*)malloc(file_size);
        read_bytes = fread(file_data, 1, file_size, fp);
        assert(read_bytes == file_size);
        fclose(fp);

        respone_size = 0;
        respone = FormatHttpResponse("200 OK", content_type, NULL, file_data, file_size, &respone_size);
        free(file_data);
        WriteUvData(client, respone, respone_size, 0, 1);
    } else {
        respone = HttpErrorPage(404, file_path);
        WriteUvData(client, respone, -1, 0, 1);
    }
}

char* HttpServerUtil::FormatHttpResponse(
    const char* status, 
    const char* content_type, 
    const char* cookie, 
    const void* content, 
    int content_length, 
    int* respone_size)
{
    int totalsize, header_size;
    char* respone;

    if (content_length < 0) {
        content_length = content ? strlen((char*)content) : 0;
    }

    totalsize = strlen(status) + strlen(content_type) + content_length + 128;
    respone = (char*)malloc(totalsize);
    if (cookie) {
        header_size = sprintf(respone, "HTTP/1.1 %s\r\n"
            "Server: Igropyr/%s\r\n"
            "Content-Type: %s; charset=utf-8\r\n"
            "Content-Length: %d\r\n"
            "Set-Cookie: %s\r\n\r\n",
            status, IGROPYR_VERSION, content_type, content_length, cookie);
    } else {
        header_size = sprintf(respone, "HTTP/1.1 %s\r\n"
            "Server: Igropyr/%s\r\n"
            "Content-Type: %s; charset=utf-8\r\n"
            "Content-Length: %d\r\n\r\n",
            status, IGROPYR_VERSION, content_type, content_length);
    }
    assert(header_size > 0);

    if (content) {
        memcpy(respone + header_size, content, content_length);
    }
    if (respone_size) {
        *respone_size = header_size + content_length;
    }

    return respone;
}

std::string HttpServerUtil::HandleStatusCode(
    int code) {
    switch (code) {
    case 200: return("200 OK"); break;
    case 301: return("301 Moved Permanently"); break;
    case 302: return("302 Found"); break;
    case 403: return("403 Forbidden"); break;
    case 404: return("404 Not Found"); break;
    case 500: return("500 Internal Server Error"); break;
    case 502: return("502 Bad Gateway"); break;
    case 504: return("504 Gateway Timeout"); break;
    case 100: return("100 Continue"); break;
    case 101: return("101 Switching Protocals"); break;
    case 201: return("201 Created"); break;
    case 202: return("202 Accepted"); break;
    case 203: return("203 Non-authoritative Information"); break;
    case 204: return("204 No Content"); break;
    case 205: return("205 Reset Content"); break;
    case 206: return("206 Partial Content"); break;
    case 300: return("300 Multiple Choices"); break;
    case 303: return("303 See Other"); break;
    case 304: return("304 Not Modified"); break;
    case 305: return("305 Use Proxy"); break;
    case 306: return("306 Unused"); break;
    case 307: return("307 Temporary Redirect"); break;
    case 400: return("400 Bad Request"); break;
    case 401: return("401 Unauthorized"); break;
    case 402: return("402 Payment Required"); break;
    case 405: return("405 Method Not Allowed"); break;
    case 406: return("406 Not Acceptable"); break;
    case 407: return("407 Proxy Authentication Required"); break;
    case 408: return("408 Request Timeout"); break;
    case 409: return("409 Conflict"); break;
    case 410: return("410 Gone"); break;
    case 411: return("411 Length Required"); break;
    case 412: return("412 Precondition Failed"); break;
    case 413: return("413 Request Entity Too Large"); break;
    case 414: return("414 Request-url Too Long"); break;
    case 415: return("415 Unsupported Media Type"); break;
    case 416: return("416 Requested Range Not Satisfiable"); break;
    case 417: return("417 Expectation Failed"); break;
    case 501: return("501 Not Implemented"); break;
    case 503: return("503 Service Unavailable"); break;
    case 505: return("505 HTTP Version Not Supported"); break;
    default: return("200 OK"); break;
    }
}

std::string HttpServerUtil::HandleContentType(
    const char* postfix) {
    if (strcmp(postfix, "html") == 0 || strcmp(postfix, "htm") == 0) {
        return "text/html";
    } else if (strcmp(postfix, "js") == 0) {
        return "text/javascript";
    } else if (strcmp(postfix, "css") == 0) {
        return "text/css";
    } else if (strcmp(postfix, "jpeg") == 0 || strcmp(postfix, "jpg") == 0) {
        return "image/jpeg";
    } else if (strcmp(postfix, "png") == 0) {
        return "image/png";
    } else if (strcmp(postfix, "gif") == 0) {
        return "image/gif";
    } else if (strcmp(postfix, "txt") == 0) {
        return "text/plain";
    } else {
        return "application/octet-stream";
    }
}

char* HttpServerUtil::HttpErrorPage(
    int error_code, 
    const char* error_info) {
    char* respone;
    std::string error = HandleStatusCode(error_code);
    char buffer[1024];
    snprintf(buffer, 
        sizeof(buffer), 
        "<html><head><title>%s</title></head><body bgcolor='white'><center><h1>%s</h1></center><hr><center>Igropyr/%s</center><p>%s</p></body></html>", 
        error, 
        error, 
        IGROPYR_VERSION, 
        error_info);
    return FormatHttpResponse(error.c_str(), "text/html", NULL, buffer, -1, NULL);
}

char* HttpServerUtil::HttpResponse(
    const int code, 
    const char* content_type, 
    const char* cookie, 
    const char* content) {
    std::string status = HandleStatusCode(code);
    return FormatHttpResponse(status.c_str(), content_type, cookie, content, -1, NULL);
}

int HttpServerUtil::HttpHandleRequest(
    igr_res response_get, 
    igr_res response_post) {
    HttpServer::Instance()->res_get() = response_get;
    HttpServer::Instance()->res_post() = response_post;
    return 1;
}


int HttpServerUtil::HttpPar(
    char* router_info, 
    char* path_info) {
    char* p1 = router_info + 1;
    char* p2 = path_info + 1;

    while(true) {
        if (*p1 != '*') {
            if (*p1 != *p2) {
                p1 = NULL;
                p2 = NULL;
                return 0;
                break;
            } else {
                if (*p1 == '\0') {
                    p1 = NULL;
                    p2 = NULL;
                    return 1;
                    break;
                } else {
                    p1++;
                    p2++;
                }
            }
        } else {
            p1++;

            if (*p1 == '\0') {
                p1 = NULL;
                p2 = NULL;
                return 1;
                break;
            } else {
                for (; *p2 != '/'; p2++) {
                }
            }
        }
    }

}

char* HttpServerUtil::HttpHeaderParser(
    char* http_header, 
    char* key) {
    char* begin = strstr(http_header, key);
    if (begin) {
        begin = begin + strlen(key);
        begin++;
        while (isspace(*begin))
            begin++;
        char* end;
        for (end = begin + 1; *end != '\r'; end++) {
        }
        *end = '\0';
        return begin;
    } else {
        return const_cast<char*>("");
    }
}

char* HttpServerUtil::HttpPathParser(
    char* path, 
    int i)
{
    char* begin = path;
    char* end;
    int n = 0;

    while(true) {
        if (*begin == '/') {
            if (n == i) {
                begin++;
                end = begin;
                i = 0;
            } else {
                begin++;
                n++;
            }

        } else if (i == 0) {
            if (*end == '/') {
                *end = '\0';
                break;
            } else if (*end == '\0') {
                break;
            } else {
                end++;
            }
        } else {
            if (*begin == '\0') {
                begin = const_cast<char*>("");
                break;
            } else {
                begin++;
            }
        }
    }

    return begin;
}