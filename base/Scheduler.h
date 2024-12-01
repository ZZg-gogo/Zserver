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
    virtual ~Scheduler();

    const std::string& getName() const
    {
        return schedulerName_;
    }
    //设置当前线程的携程调度器指针
    void SetCurrentScheduler();

    //启动协程调度器
    void start();
    //退出协程调度器
    void stop();

    //线程执行run方法去调度携程
    void run();

       

    template <typename T>
    void addJob(T t, int id = -1)
    {
        job j(t, id);
        if (j.fiber || j.fun)
        {
            Mutex::Lock lock(mutex_);
            tasks_.push_back(j);
            tickle();
        } 
    }

    template <typename T>
    void addMultiJob(const T& store)
    {
        Mutex::Lock lock(mutex_);

        for (auto& i : store)
        {
            job j(i, -1);
            tasks_.push_back(i);
        }
        tickle();
    }

protected:
    virtual void tickle();
    //协程空闲执行此函数
    virtual void idle(); 
    //是否已经完全退出了
    virtual bool isStop();
public:
    //获取当前正在运行的携程调度器
    static Scheduler* GetCurrentScheduler();
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

        bool isVaild()
        {
            return fiber || fun;
        }
        
        ~job(){}
    };

private:
    Mutex mutex_;
    std::vector<Thread::ptr> threads_;  //保存所有的线程
    std::string schedulerName_;         //调度器名称
    std::list<job> tasks_;              //任务队列里面所有的任务

protected:
    std::vector<int> threadIds_;        //保存所有的线程id
    size_t threadNum_;                  //线程数
    size_t avtiveThreadCount_;          //活跃线程数量
    size_t idleThreadCount_;            //空闲线程数量
    bool isStop_;                       //是否停止
    int rootThreadId_;                  //主线程id
    Fiber::ptr rootFiber_;              
};




}//end namespace