#pragma once
#include <queue>
#include <mutex>
#include <memory>

namespace top
{
 template<typename ValueType>
class QueueTemplate {
    typedef std::shared_ptr<ValueType> ValueTypeSptr;
public:
    virtual bool Init() {}
    virtual void UnInit() {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!queue_.empty()) {
            if (!Release(queue_.back())) {
                continue;
            }
            queue_.pop();
        }
    }
    virtual bool Push(
        const ValueTypeSptr in_value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (NULL == in_value) {
            return false;
        }
        queue_.push(in_value);
        return true;
    }
    virtual bool Pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.pop();
        return true;
    }
protected:
    virtual bool Release(ValueTypeSptr&) = 0;
    QueueTemplate() {}
    virtual ~QueueTemplate() {}
    mutable std::mutex mutex_;
    std::queue<ValueTypeSptr> queue_;
};
}  //  namespace top