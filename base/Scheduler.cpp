#include "Scheduler.h"
#include "macro.h"
#include "util.h"
#include "Hook.h"
namespace BASE
{


//当前线程所持有的那个调度器
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
    mutex_(),
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
        rootFiber_.reset(new Fiber{std::bind(&Scheduler::run, this), 0, true});
        SchedulerFiber = rootFiber_;

        rootThreadId_ = BASE::getThreadId();
        threadIds_.push_back(rootThreadId_);
    }
    else
    {   //当前线程 不参与
        rootThreadId_ = -1;
    }
}


Scheduler::~Scheduler()
{
    if (!isStop_)
    {
        stop();
    }
    

    if (GetCurrentScheduler() == this)
    {
        CurScheduler = nullptr;
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

//退出协程调度器
void Scheduler::stop()
{
    LOG_INFO(LOG_ROOT)<<"Scheduler::stoping";
    isStop_ = true;
    if (rootThreadId_ == -1)
    {
        ZZG_ASSERT(GetCurrentScheduler() != this);
    }
    else
    {
        ZZG_ASSERT(GetCurrentScheduler() == this);
    }

    for (int i = 0; i < threadNum_; i++)
    {
        
        tickle();
        //sleep(1);
    }
    
    if (rootFiber_)
    {
        tickle();
    }

    if (rootFiber_)
    {
        rootFiber_->call();
    }


    
    std::vector<Thread::ptr> thrs;
    {
        Mutex::Lock lock(mutex_);
        thrs.swap(threads_);
    }
    

    for (auto& t : thrs)
    {
        t->join();
    }
    
}

bool Scheduler::isStop()
{
    Mutex::Lock lock(mutex_);
    return isStop_ && tasks_.empty() && avtiveThreadCount_ == 0;
}


void Scheduler::tickle()
{
    LOG_INFO(LOG_ROOT)<<" Scheduler::tickle";
}

void Scheduler::idle()
{
    LOG_INFO(LOG_ROOT)<<" Scheduler::idle";
    while(!isStop()) 
    {
        BASE::Fiber::YieldToSuspended();
    }
}


//
void Scheduler::run()
{
    SetHook(true);
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
                if (it->threadId == -1 ||  threadIds_[it->threadId] == curThreadId)
                {
                    j = *it;
                    tasks_.erase(it);
                    break;
                }
            }

            if (j.isVaild())
                ++avtiveThreadCount_;
        }


        if (j.fiber)
        {
            cbFiber = j.fiber;
        }
        else if (j.fun) 
        {
            if (cbFiber)
            {
                cbFiber->reset(j.fun);
            }
            else
            {
                cbFiber.reset(new Fiber(j.fun));
            }
            
        }

        if (j.isVaild())
        {
            //运行当前的协程
            cbFiber->resume();
            --avtiveThreadCount_;

            //如果说当前协程的状态是就绪态 那么还要继续加入到队列中
            if (cbFiber->getState() == Fiber::State::READY)
            {
                addJob(cbFiber);
                cbFiber.reset();
            }
            else if (cbFiber->getState() != Fiber::State::ERROR 
                        && cbFiber->getState() != Fiber::State::TERM)
            {
                //协程被挂起
                cbFiber->setState(Fiber::State::SUSPENDED);
                cbFiber.reset();
            } 
            else if (cbFiber->getState() == Fiber::State::ERROR 
                        || cbFiber->getState() == Fiber::State::TERM)
            {
                cbFiber->reset(nullptr);
            }
        }
        //没有拿到任务
        else 
        {
            if (idleFiber->getState() == Fiber::State::TERM)
            {
                LOG_INFO(LOG_ROOT)<<"Scheduler::run break";
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
    

    LOG_INFO(LOG_ROOT)<<"Scheduler::run Thread Exit curThreadId="<<curThreadId;
    
}

}