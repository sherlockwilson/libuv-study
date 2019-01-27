#pragma once

#include "uv_http_util_def.h"

class HttpServerCallBack {
public:
    static void OnConnection(
        uv_stream_t* server, 
        int status);

    static void OnUvAlloc(
        uv_handle_t* handle,
        size_t suggested_size,
        uv_buf_t* buf);

    static void OnUvRead(
        uv_stream_t* client,
        ssize_t nread,
        const uv_buf_t* buf);

    static void AfterUvCloseClient(
        uv_handle_t* client);

    static void AfterUvWrite(
        uv_write_t* w, 
        int status);
};