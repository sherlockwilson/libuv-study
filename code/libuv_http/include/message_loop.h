#pragma once

#include <uv.h>

#include <memory>
#include <future>

#include "manager_template.h"
#include "timer.h"
#include "http_util_def.h"
#include "url_cmd_manager.h"
#include "http_server.h"
#include "http_util.h"

template<typename MessageType>
class MessageLoopBase :
    public ManagerTemplate<
    int32_t,
    MessageType> {
public:
    MessageLoopBase()
        :message_id_(0) {
    }
    virtual ~MessageLoopBase() {}
    virtual bool Init() {
        int32_t interval = 1;
        sptr_timer_ = std::make_shared<Timer>();
        sptr_timer_->StartTimer(
            interval,
            std::bind(
                &MessageLoopBase::Run,
                this));
        return true;
    }

    virtual void UnInit() {
        sptr_timer_->Expire();
    }

    virtual bool PostMsg(
        const MessageType session) = 0;
protected:
    virtual void Exec(const MessageType session) = 0;

    virtual void Run() {
        for (auto& pair : this->map_) {
            Exec(pair.second);
            this->DeleteKey(pair.first);
        }
    }
    std::atomic<int32_t> message_id_;
    std::shared_ptr<Timer> sptr_timer_;
};

class MessageLoopForGet :
    public MessageLoopBase<HttpSessionSptr> {
public:
    static MessageLoopForGet* Instance() {
        static MessageLoopForGet ins;
        return &ins;
    }

    virtual bool PostMsg(
        const HttpSessionSptr session);
protected:
    virtual void Exec(const HttpSessionSptr session);
    MessageLoopForGet() {}
    ~MessageLoopForGet() {}
};