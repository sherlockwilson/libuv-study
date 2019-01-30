#include "cron_task_manager.h"

namespace top {

bool CronTaskManager::AddCronTask(
    std::function<void()> task,
    const int32_t interval) {
    auto timer = std::make_shared<Timer>();
    timer->StartTimer(interval, task);
    if (!Push(timer)) {
        return false;
    }
    return true;
}

bool CronTaskManager::Release(TimerSptr& timer) {
    timer->Expire();
}

}  //  namespace top