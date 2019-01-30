#include "http_client_session_manager.h"

namespace top {

HttpClientSessionManager* HttpClientSessionManager::Instance() {
    static HttpClientSessionManager ins;
    return &ins;
}

bool HttpClientSessionManager::Init() {
    int32_t interval = 1;
    sptr_timer_ = std::make_shared<Timer>();
    sptr_timer_->StartTimer(
        interval,
        std::bind(
            &HttpClientSessionManager::Run,
            this));
    return true;
}

void HttpClientSessionManager::UnInit() {
    sptr_timer_->Expire();
}

void HttpClientSessionManager::Run() {
    for (auto& pair : map_) {
        std::string& post_data = 
            pair.second->content;
        if(!pair.second->IsExpire()) {
            continue;
        }
        //TODO:
    }
}

bool HttpClientSessionManager::Insert(
    uv_stream_t* in_key,
    const HttpSessionSptr in_value) {
    if (NULL == in_key) {
        return false;
    }

    if (!AddData(in_key, in_value)) {
        return false;
    }

    return true;
}

bool HttpClientSessionManager::Mod(
    uv_stream_t* in_key,
    const HttpSessionSptr in_value) {
    if (NULL == in_key) {
        return false;
    }

    if (!ModData(in_key, in_value)) {
        return false;
    }

    return true;
}

bool HttpClientSessionManager::Contains(
    uv_stream_t* in_key) {
    if (NULL == in_key) {
        return false;
    }
    if (!HaveKey(in_key)) {
        return false;
    }

    return true;
}

void HttpClientSessionManager::Delete(
    uv_stream_t* in_key) {
    if (NULL == in_key) {
        return ;
    }
    DeleteKey(in_key);
}

bool HttpClientSessionManager::Find(
    uv_stream_t* in_key,
    HttpSessionSptr& out_value) {
    if (NULL == in_key) {
        return false;
    }
    if (!FindData(in_key, out_value)) {
        return false;
    }

    return true;
}

}  //  namespace top