#include "Scheduler.h"
#include "macro.h"
#include "util.h"

namespace BASE
{



static thread_local Scheduler* CurScheduler = nullptr;
//这个调度携程 也就是每个线程的主协程
static thread_local Fiber::ptr SchedulerFiber = nullptr;


void Scheduler::SetCurrentScheduler()
{
    CurScheduler = this;
}

//获取当前正在运行的携程调度器
Scheduler* Scheduler::GetCurrentScheduler()
{
    return CurScheduler;
}
//获取主协程
Fiber::ptr Scheduler::GetMainFiber()
{
    return SchedulerFiber;
}







Scheduler::Scheduler(size_t threadNum, bool callerJoin, const std::string& name) :
    threadNum_(threadNum),
    schedulerName_(name),
    avtiveThreadCount_(0),
    idleThreadCount_(0),
    isStop_(true),
    rootThreadId_(),
    rootFiber_(nullptr)
{
    ZZG_ASSERT(threadNum >= 1)

    //如果当前构造调度器的线程 也要加入线程池当中
    if (callerJoin)
    {
        Fiber::GetCurFiber();
        --threadNum_;

        //当前的线程里面没有协程调度器
        ZZG_ASSERT(CurScheduler == nullptr)
        /* 这个线程的主协程不再是MainFiber
            而是SchedulerFiber
        */
        SetCurrentScheduler();
        rootFiber_.reset(new Fiber{std::bind(&Scheduler::run, this)});
        SchedulerFiber = rootFiber_;

        rootThreadId_ = BASE::getThreadId();
        threadIds_.push_back(rootThreadId_);
    }
    else
    {   //当前线程 不参与
        rootThreadId_ = -1;
    }
}


void Scheduler::start()
{
    if (!isStop_)
    {
        return;
    }

    isStop_ = false;
    
    //线程池是空的
    ZZG_ASSERT(threads_.empty())
    
    threads_.resize(threadNum_);

    for (size_t i = 0; i < threadNum_; i++)
    {
        threads_[i].reset(new Thread{std::bind(std::bind(&Scheduler::run, this)), "threadName_" + std::to_string(i)}); 
        threadIds_.push_back(threads_[i]->getId());
    }
}

void Scheduler::stop()
{

}


void Scheduler::tickle()
{

}


//
void Scheduler::run()
{
    SetCurrentScheduler();
    pid_t curThreadId = BASE::getThreadId();

    //当这个线程不是 构造调度器的线程的话
    if (rootThreadId_ != curThreadId)
    {
        //先要构造出来 主协程
        SchedulerFiber = Fiber::GetCurFiber();
    }
    
    Fiber::ptr idleFiber(new Fiber(std::bind(&Scheduler::idle, this)));
    Fiber::ptr cbFiber;
    
    job j;
    while (true)
    {
        j.reset();

        {
            Mutex::Lock lock(mutex_);

            for (auto it = tasks_.begin(); it !=  tasks_.end(); ++it)
            {
                //如果当前的任务没有指定线程 或者指定当前线程运行的话
                if (it->threadId == -1 ||  it->threadId == curThreadId)
                {
                    j = *it;
                    tasks_.erase(it);
                }
            }
        }


        if (j.fiber)
        {
        }
        else if (j.fun) 
        {
            //functional-->fiber
            j.fiber.reset(new Fiber(j.fun));  
        }

        if (j.isVaild())
        {
            ++avtiveThreadCount_;
            //运行当前的协程
            j.fiber->resume();
            --avtiveThreadCount_;

            //如果说当前协程的状态是就绪态 那么还要继续加入到队列中
            if (j.fiber->getState() == Fiber::State::READY)
            {
                addJob(j.fiber);
            }
            else if (j.fiber->getState() != Fiber::State::ERROR 
                        && j.fiber->getState() != Fiber::State::TERM)
            {
                //协程被挂起
                j.fiber->setState(Fiber::State::SUSPENDED);
            } 
            else if (j.fiber->getState() == Fiber::State::ERROR 
                        || j.fiber->getState() == Fiber::State::TERM)
            {
                j.fiber->reset(nullptr);
            }
        }
        //没有拿到任务
        else 
        {
            if (idleFiber->getState() == Fiber::State::TERM)
            {
                break;
            }
            ++idleThreadCount_;
            idleFiber->resume();

            if (idleFiber->getState() != Fiber::State::ERROR 
                        && idleFiber->getState() != Fiber::State::TERM)
            {
                //协程被挂起
                idleFiber->setState(Fiber::State::SUSPENDED);
            } 

            --idleThreadCount_;
        }  
    }
    
}

}