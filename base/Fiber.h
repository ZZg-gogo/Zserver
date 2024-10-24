#pragma once

#include <functional>
#include <memory>
#include <ucontext.h>
#include <atomic>

#include "Lock.h"

namespace BASE
{


//这里实现的是非对称协程
class Fiber : public std::enable_shared_from_this<Fiber>
{
public:
    typedef std::shared_ptr<Fiber> ptr;
    typedef std::function<void()> FiberFun;

    enum State
    {
        INIT,       //初始化
        READY,      //就绪
        RUNNING,    //运行
        SUSPENDED   //挂起
    };
public:
    Fiber(FiberFun fun, size_t stackSize);
    ~Fiber();

    //重置协程的执行函数
    void reset(FiberFun fun);

    void resume();  //切换到当前协程执行
    void yield();   //当前协程让出执行权




public:
    //获得当前运行的协程
    static Fiber::ptr getCurFiber();
    //协程切换到后台 并且设置为READY
    static void YieldToReady();
    //协程切换到后台 并且设置为SUSPENDED
    static void YieldToSuspended();
    //ucontext协程函数
    static void MainFun();

    static uint64_t getFiberCount() 
    {
        return FibersCount;
    }

    static uint64_t nextFiberId()
    {
        ++FibersCount;
        return ++FiberId;
    }

private:
    uint64_t fiberId_;                  //协程id
    size_t stackSize_;                  //栈大小
    State state_;                       //当前状态
    ucontext_t ucontext_;               //运行上下文
    char * stackPoint_;                 //指向栈空间
    FiberFun cb_;                       //协程函数
private:
    static std::atomic_int64_t FibersCount;        //统计当前协程的数量
    static std::atomic_int64_t FiberId; //分配协程id
};







}//end namespace