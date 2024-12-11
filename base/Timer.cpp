#include "Timer.h"
#include "util.h"

namespace BASE
{

Timer::Timer(uint64_t ms, Callback cb, bool repeat, TimerManager * manager) :
    ms_(ms),
    callCbTime_(GetTimestampMs() + ms),
    cb_(cb),
    repeat_(repeat),
    manager_(manager)
{

}


 TimerManager::TimerManager()
 {

 }
TimerManager::~TimerManager()
{

}

//添加一个定时器
Timer::ptr TimerManager::addTimer(uint64_t ms, Callback cb, bool repeat)
{
    Mutex::Lock lock(mutex_);
    Timer::ptr timer(new Timer{ms, cb, repeat, this});
    auto it =  timers_.insert(timer).first;
    
    //加入了一个最小的定时器
    if (timers_.begin() == it)
    {
        minTimerChange();
    }
    

    return *it;
}


static void ConditionCallback(std::weak_ptr<void> weak, Timer::Callback cb)
{
    if (weak.lock())
    {
        cb();
    }
    
}

//条件定时器 当weak的智能指针还存在的时候 那么才会执行定时器的回调
Timer::ptr TimerManager::addConditionTimer(uint64_t ms, Callback cb,
    std::weak_ptr<void> weak, bool repeat)
{
    Mutex::Lock lock(mutex_);
    return addTimer(ms, std::bind(ConditionCallback, weak, cb), repeat);
}



}