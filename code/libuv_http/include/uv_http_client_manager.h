#pragma once

#include <uv.h>

#include <memory>

#include "manager_template.h"
#include "timer.h"

enum OperateType {
    POST = 1,
    GET=2
};

struct HttpSession {
    HttpSession() {}
    HttpSession(
        const OperateType in_operate_type,
        const int32_t in_content_len,
        const std::string& in_local_content)
        :operate_type(in_operate_type),
        content_len(in_content_len),
        local_content(in_local_content)
    {}
    ~HttpSession() {}

    bool IsExpire() const {
        return local_content.length() >= content_len;
    }
    OperateType operate_type;
    int32_t content_len;
    std::string local_content;
};

typedef std::shared_ptr<HttpSession> HttpSessionSptr;

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