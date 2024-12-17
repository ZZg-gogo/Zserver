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


//取消一个定时器
bool Timer::cancel()
{
    Mutex::Lock lock(manager_->mutex_);
    auto it = manager_->timers_.find(shared_from_this());
    manager_->timers_.erase(it);

    return true;
}
//刷新定时器的时间
bool Timer::refresh()
{
    Mutex::Lock lock(manager_->mutex_);
    auto it = manager_->timers_.find(shared_from_this());

    if (it == manager_->timers_.end())
    {
        return false;
    }
    
    manager_->timers_.erase(it);
    callCbTime_ = GetTimestampMs() + ms_;
    manager_->timers_.insert(shared_from_this());
    return true;
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

//条件定时器的辅助函数
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
    return addTimer(ms, std::bind(ConditionCallback, weak, cb), repeat);
}


bool TimerManager::empty()
{
    Mutex::Lock lock(mutex_);
    return 0 == timers_.size();
}

uint64_t TimerManager::getNextTime()
{
    Mutex::Lock lock(mutex_);

    if (timers_.empty())
    {
        return  UINT64_MAX;
    }
    else
    {
        uint64_t now = GetTimestampMs();
        uint64_t nextTime = timers_.begin()->get()->getNextCallbackTime();
        if (nextTime <= now)    //不知道为什么时间晚了
        {
            return 0;   //立即执行
        }
        else
        {
            return nextTime - now;  //还需要等待这么长的时间
        }
    }
}

void TimerManager::getExpireCallback(std::vector<Callback> & cbs)
{
    Mutex::Lock lock(mutex_);

    uint64_t now = GetTimestampMs();

    std::vector<Timer::ptr> expireTimer;

    auto it = timers_.begin(); 
    for (; timers_.end() != it; ++it)
    {
        if ((*it)->getNextCallbackTime() <= now)
        {
            cbs.push_back((*it)->cb_);
            expireTimer.push_back(*it);
        } 
        else
        {
            break;
        }
    }

    timers_.erase(timers_.begin(), it);
    
    for (auto& i : expireTimer)
    {
        if (i->repeat_)
        {
            i->callCbTime_ = now + i->ms_;
            timers_.insert(i);
        }
    }
    
}


}