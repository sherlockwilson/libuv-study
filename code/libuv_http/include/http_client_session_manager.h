#pragma once

#include <uv.h>

#include <memory>

#include "manager_template.h"
#include "timer.h"
#include "http_util_def.h"



class HttpClientSessionManager :
    public ManagerTemplate<
    uv_stream_t*,
    HttpSessionSptr> {
public:
    static HttpClientSessionManager* Instance();
    bool Init();
    void UnInit();
    bool Insert(
        uv_stream_t* in_key,
        const HttpSessionSptr in_value);
    bool Mod(
        uv_stream_t* in_key,
        const HttpSessionSptr in_value);
    bool Contains(
        uv_stream_t* in_key);
    void Delete(
        uv_stream_t* in_key);
    bool Find(
        uv_stream_t* in_key,
        HttpSessionSptr& out_value);
private:
    void Run();
    HttpClientSessionManager()
        :sptr_timer_() {
    }
    ~HttpClientSessionManager() {}
    std::shared_ptr<Timer> sptr_timer_;
};