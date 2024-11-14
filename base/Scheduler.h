#pragma once

#include <functional>
#include <memory>
#include <list>
#include "Thread.h"
#include "Fiber.h"
#include "NoCopyable.h"

namespace BASE
{

//协程调度器
class Scheduler : NoCopyable
{
public:
    typedef std::shared_ptr<Scheduler> ptr;
public:
    /*
    协程调度器将任务分配给threadNum个线程去运行
    callerJoin调用这个构造函数的线程是否也算在内
    name 线程名
    */
    Scheduler(size_t threadNum = 1, bool callerJoin = true, const std::string& name = "");
    virtual ~Scheduler(){}

    const std::string& getName() const
    {
        return schedulerName_;
    }

    void start();

    void stop();



    template <typename TASK>
    void addJob(TASK t, int id = -1)
    {
        Mutex::Lock lock(mutex_);
        job j(t, id);

        if (j.fiber || j.fun)
        {
            tasks_.push_back(j);
            tickle();
        } 
    }

protected:
    void tickle();

public:
    //获取当前正在运行的携程调度器
    static Scheduler::ptr GetCurrentScheduler();
    //获取主协程
    static Fiber::ptr GetMainFiber();

private:
    /*
    任务队列里面可以塞入协程or仿函数
    任务也可以放入特定的线程id中进行运行
    */
    struct job
    {
        std::function<void()> fun;
        Fiber::ptr fiber;
        int threadId;
        job(Fiber::ptr f, int id = -1)
        {
            fiber = f;
            threadId = id;
        } 

        job(std::function<void()> f, int id = -1)
        {
            fun = f;
            threadId = id;
        } 

        void reset()
        {
            fiber = nullptr;
            fun = nullptr;
            threadId = -1;
        }

        job() :
            fiber(nullptr),
            fun(nullptr),
            threadId(-1)
        {

        }

        ~job(){}
    };

private:
    Mutex mutex_;
    size_t threadNum_;                  //线程数
    std::vector<Thread::ptr> threads_;  //保存所有的线程
    std::string schedulerName_;         //调度器名称
    std::list<job> tasks_;
};




}//end namespace