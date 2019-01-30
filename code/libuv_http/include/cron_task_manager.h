#pragma once

#include <stdint.h>

#include "queue_template.h"
#include "timer.h"

namespace top {
class Timer;
typedef std::shared_ptr<Timer> TimerSptr;
}

namespace top {

class CronTaskManager :
    public QueueTemplate<Timer> {
public:
    static CronTaskManager* Instance() {
        static CronTaskManager ins;
        return &ins;
    }
    bool AddCronTask(
        std::function<void()> task,
        const int32_t interval);
private:
    CronTaskManager() {}
    virtual ~CronTaskManager() {}
    virtual bool Release(TimerSptr& timer);
};

}  //  namespace top