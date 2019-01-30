#pragma once

#include <uv.h>

#include <memory>
#include <queue>

#include "manager_template.h"
#include "http_util_def.h"
#include "timer.h"

namespace top
{
template<typename MessageType>
class MessageLoopBase {
    typedef std::shared_ptr<MessageType> MessageTypeSptr;
public:
    MessageLoopBase() {}
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
        const MessageTypeSptr session) {
        if (NULL == session) {
            return false;
        }
        msg_queue_.push(session);
        return true;
    }
protected:
    virtual void Exec(const MessageTypeSptr session) = 0;

    virtual void Run() {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!msg_queue_.empty()) {
            Exec(msg_queue_.back());
            msg_queue_.pop();
        }
    }
    std::mutex mutex_;
    std::atomic<int32_t> message_id_;
    std::shared_ptr<Timer> sptr_timer_;
    std::queue<MessageTypeSptr> msg_queue_;
};

class MessageLoopForGet :
    public MessageLoopBase<HttpSession> {
public:
    static MessageLoopForGet* Instance() {
        static MessageLoopForGet ins;
        return &ins;
    }
protected:
   void Exec(const HttpSessionSptr session);
private:
    MessageLoopForGet() {}
    ~MessageLoopForGet() {}
};
}  //  namespace top