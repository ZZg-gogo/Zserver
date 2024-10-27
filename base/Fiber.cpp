#include "Fiber.h"
#include "Config.h"
#include "macro.h"

namespace BASE
{

static thread_local Fiber* CurFiber = nullptr;
static thread_local Fiber::ptr MainFiber = nullptr;

std::atomic_int64_t Fiber::FibersCount = 0;
std::atomic_int64_t Fiber::FiberId = 0;

static ConfigVar<uint32_t>::ptr StackSizeCfg  =  ConfigVar<uint32_t>::Create("Fiber.StackSize", "协程栈大小", 1024*8);

//设置当前线程的运行协程
void Fiber::SetCurFiber(Fiber * f)
{
    CurFiber = f;
}

//获得当前运行的协程
Fiber::ptr Fiber::GetCurFiber()
{
    if (CurFiber)
    {
        return CurFiber->shared_from_this();
    }


    Fiber::ptr mainFiber {new Fiber()};
    ZZG_ASSERT(CurFiber == mainFiber.get())
    MainFiber = mainFiber;

    return CurFiber->shared_from_this();
}

uint64_t Fiber::getCurFiberId()
{
    if (CurFiber)
    {
        return CurFiber->fiberId_;
    }
    
}

//协程切换到后台 并且设置为READY
void Fiber::YieldToReady()
{
    Fiber::ptr cur = GetCurFiber(); //得到当前的协程
    ZZG_ASSERT(cur.get() != MainFiber.get())
    cur->state_ = State::READY;
    cur->yield();
}


//协程切换到后台 并且设置为SUSPENDED
void Fiber::YieldToSuspended()
{
    Fiber::ptr cur = GetCurFiber(); //得到当前的协程
    ZZG_ASSERT(cur.get() != MainFiber.get())
    cur->state_ = State::SUSPENDED;
    cur->yield();
}

//协程切换到后台 并且设置为结束
void Fiber::YieldToTerm()
{
    Fiber::ptr cur = GetCurFiber(); //得到当前的协程
    ZZG_ASSERT(cur.get() != MainFiber.get())
    cur->state_ = State::TERM;
    cur->yield();
}

void Fiber::MainFun()
{
    Fiber::ptr cur = GetCurFiber(); //得到当前的协程
    ZZG_ASSERT(cur && cur->state_ == State::RUNNING)
    try
    {
        cur->cb_();
        cur->cb_ = nullptr;
        cur->state_ = State::TERM;
    }
    catch(const std::exception& e)
    {
        cur->state_ = State::ERROR;
        LOG_ERROR(LOG_ROOT) << e.what();
    }
    catch(...)
    {
        cur->state_ = State::ERROR;
        LOG_ERROR(LOG_ROOT) << "Something error in Fiber id="<<cur->fiberId_;
    }
    
    //YieldToTerm();
}

//每个线程的主协程直接就是运行状态
Fiber::Fiber() : 
    fiberId_(nextFiberId()),    
    stackSize_(0),
    state_(State::RUNNING),
    ucontext_(),
    stackPoint_(nullptr),
    cb_(nullptr)
{
    SetCurFiber(this);
    if (getcontext(&ucontext_))
    {
        LOG_ERROR(LOG_ROOT)<<"New Fiber Create Error Reason = getcontext";
        return;
    }

    LOG_INFO(LOG_ROOT)<<"New Main Fiber Create Succ Fiber Id = "<<fiberId_<<" stackSize = "<<stackSize_;
}


Fiber::Fiber(FiberFun fun, size_t stackSize) : 
    fiberId_(nextFiberId()),    
    stackSize_(stackSize),
    state_(State::INIT),
    ucontext_(),
    stackPoint_(nullptr),
    cb_(fun)
{
    if (!stackSize_)
    {
        stackSize_ = StackSizeCfg->getVal();
    }
    
    if (getcontext(&ucontext_))
    {
        LOG_ERROR(LOG_ROOT)<<"New Fiber Create Error Reason = getcontext";
        return;
    }


    stackPoint_ = new char[stackSize_];
    if (nullptr == stackPoint_)
    {
        LOG_ERROR(LOG_ROOT)<<"New Fiber Create Error Reason = new";
        return;
    }

    ucontext_.uc_flags = 0;
    ucontext_.uc_stack.ss_size = stackSize_;
    ucontext_.uc_stack.ss_sp = stackPoint_;
    ucontext_.uc_link = &MainFiber->ucontext_;

    makecontext(&ucontext_, MainFun, 0);

    LOG_INFO(LOG_ROOT)<<"New Fiber Create Succ Fiber Id = "<<fiberId_<<" stackSize = "<<stackSize_;
}

Fiber::~Fiber()
{
    
    --FibersCount;
    LOG_INFO(LOG_ROOT)<<"New Fiber destroy  Fiber Id = "<<fiberId_;
    if (stackPoint_)//子协程
    {
        ZZG_ASSERT(state_ == State::INIT || state_ == State::TERM || state_ == State::ERROR)
        delete []stackPoint_;
        stackPoint_ = nullptr;
    }
    else//主协程 一直是运行态并且没有回调函数
    {
        ZZG_ASSERT(state_ == State::RUNNING && !cb_)
    }
    
}

void Fiber::reset(FiberFun fun)
{
    //必须有栈 必须处在初始化或者结束状态
    ZZG_ASSERT(stackPoint_)
    ZZG_ASSERT(state_ == State::INIT || state_ == State::TERM || state_ == State::ERROR)
    cb_ = fun;

    if (getcontext(&ucontext_))
    {
        LOG_ERROR(LOG_ROOT)<<"Fiber reset Error Reason = getcontext";
        return;
    }

    ucontext_.uc_flags = 0;
    ucontext_.uc_stack.ss_size = stackSize_;
    ucontext_.uc_stack.ss_sp = stackPoint_;
    ucontext_.uc_link = nullptr;

    makecontext(&ucontext_, MainFun, 0);
    state_ = State::INIT;
}

//切换到当前协程执行
void Fiber::resume()
{
    SetCurFiber(this);
    ZZG_ASSERT(state_ != State::RUNNING)
    state_ = State::RUNNING;
    if (swapcontext(&(MainFiber->ucontext_), &ucontext_))
    {
       LOG_ERROR(LOG_ROOT)<<"Fiber resume Error Reason = swapcontext";
       return;
    }
}

//当前协程让出执行权

void Fiber::yield()
{
    SetCurFiber(MainFiber.get());
    if (swapcontext(&ucontext_, &MainFiber->ucontext_))
    {
       LOG_ERROR(LOG_ROOT)<<"Fiber yield Error Reason = swapcontext";
       return;
    }

}



}//end namespace