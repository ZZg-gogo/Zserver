#pragma once

#include <functional>
#include <memory>
#include <set>
#include <vector>

#include "Lock.h"

namespace BASE
{

class TimerManager;

class Timer : public std::enable_shared_from_this<Timer>
{
friend class TimerManager;

public:
    typedef std::function<void()> Callback;
    typedef std::shared_ptr<Timer> ptr;

    struct Compare
    {
        bool operator()(const ptr& a, const ptr& b)
        {
            return a->getNextCallbackTime() < b->getNextCallbackTime();
        }
    };
public:
    Timer() = delete;
    ~Timer() = default;
private:
    //外部不能直接创建Timer对象
    Timer(uint64_t ms, Callback cb, bool repeat, TimerManager * manager);

    //获得下一次触发回调的时间
    uint64_t getNextCallbackTime() const
    {
        return callCbTime_;
    }
    //取消一个定时器
    bool cancel();
    //刷新定时器的时间
    bool refresh();

private:
    uint64_t ms_;               //定时器的周期
    uint64_t callCbTime_;       //下一次调用定时器的时间戳
    Callback cb_;               //回调函数
    bool repeat_;               //是否重复
    TimerManager * manager_;    //被哪一个TimerManager管理
};




class TimerManager
{
friend class Timer;
public:
    typedef std::shared_ptr<TimerManager> ptr;
    using Callback = Timer::Callback;
public:
    TimerManager();
    ~TimerManager();

    //添加一个定时器
    Timer::ptr addTimer(uint64_t ms, Callback cb, bool repeat = false);
    //条件定时器 当weak的智能指针还存在的时候 那么才会执行定时器的回调
    Timer::ptr addConditionTimer(uint64_t ms, Callback cb,
        std::weak_ptr<void> weak, bool repeat = false);
    //获取epoll要定时多长时间
    uint64_t getNextTime();
    //超时触发了 获取到那些需要执行回调的函数
    void getExpireCallback(std::vector<Callback> & cbs);
protected:
    //插入了一个到期时间最小的timer
    virtual void minTimerChange() = 0;
private:
    Mutex mutex_;
    std::set<Timer::ptr, Timer::Compare> timers_;
};




}