#pragma once

#include <uv.h>

#include <memory>

#include "manager_template.h"
#include "timer.h"

struct HttpPost {
    HttpPost() {}
    HttpPost(
        const int32_t in_content_len,
        const std::string& in_local_content)
        :content_len(in_content_len),
        local_content(in_local_content)
    {}
    ~HttpPost() {}

    bool IsExpire() const {
        return local_content.length() >= content_len;
    }

    int32_t content_len;
    std::string local_content;
};

typedef std::shared_ptr<HttpPost> HttpPostSptr;

class HttpClientManager :
    public ManagerTemplate<
    uv_tcp_t*, 
    HttpPostSptr> {
public:
    static HttpClientManager* Instance();
    bool Init(
        const int32_t in_interval);
    void UnInit();
    bool Insert(
        uv_tcp_t* in_key, 
        const HttpPostSptr in_value);
    bool Mod(
        uv_tcp_t* in_key,
        const HttpPostSptr in_value);
    bool Contains(
        uv_tcp_t* in_key);
    void Delete(
        uv_tcp_t* in_key);
private:
    void Run();
    HttpClientManager()
        :sptr_timer_() {
    }
    ~HttpClientManager() {}
    std::shared_ptr<Timer> sptr_timer_;
};