#include "uv_http_client_manager.h"
HttpClientManager* HttpClientManager::Instance() {
    static HttpClientManager ins;
    return &ins;
}

bool HttpClientManager::Init(
    const int32_t in_interval) {
    if (in_interval <= 0) {
        return false;
    }
    sptr_timer_ = std::make_shared<Timer>();
    sptr_timer_->StartTimer(
        in_interval, 
        std::bind(
            &HttpClientManager::Run, 
            this));
    return true;
}

void HttpClientManager::UnInit() {
    sptr_timer_->Expire();
}

void HttpClientManager::Run() {
    for (auto& pair : map_) {
        if(!pair.second->IsExpire()) {
            continue;
        }
        //TODO:
    }
}

bool HttpClientManager::Insert(
    uv_tcp_t* in_key,
    const HttpPostSptr in_value) {
    if (NULL == in_key) {
        return false;
    }

    if (!AddData(in_key, in_value)) {
        return false;
    }

    return true;
}

bool HttpClientManager::Mod(
    uv_tcp_t* in_key,
    const HttpPostSptr in_value) {
    if (NULL == in_key) {
        return false;
    }

    if (!ModData(in_key, in_value)) {
        return false;
    }

    return true;
}

bool HttpClientManager::Contains(
    uv_tcp_t* in_key) {
    if (NULL == in_key) {
        return false;
    }
    if (!HaveKey(in_key)) {
        return false;
    }

    return true;
}

void HttpClientManager::Delete(
    uv_tcp_t* in_key) {
    if (NULL == in_key) {
        return ;
    }
    DeleteKey(in_key);
}