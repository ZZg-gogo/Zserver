#include "IoManager.h"
#include <sys/epoll.h>

#include "macro.h"
#include "Logger.h"

namespace BASE
{


IoManager * IoManager::GetThis()
{
    dynamic_cast<IoManager*>(Scheduler::GetCurrentScheduler());
}


void IoManager::FdContext::triggerEvent(EventType type)
{
    EventContext& ctx = getContext(type);

    event = static_cast<EventType>(event & ~type);
    
    if (ctx.cb)
    {
        ctx.scheduler->addJob(ctx.cb);
    }
    else if (ctx.fiber)
    {
        ctx.scheduler->addJob(ctx.fiber);
    }

    ctx.scheduler = nullptr;
}

void IoManager::FdContext::resetContext(EventContext& ctx)
{
    ctx.cb = nullptr;
    ctx.fiber.reset();
    ctx.scheduler = nullptr;
}

IoManager::FdContext::EventContext& IoManager::FdContext::getContext(EventType event)
{
    if (event & READ)
    {
        return readCallback;
    }
    else if (event & WRITE)
    {
        return writeCallback;
    }
    else
    {
        LOG_ERROR(LOG_ROOT)<<"IoManager::FdContext::getContext event="<<event;
        throw std::invalid_argument("getContext invalid event");
    }
}


IoManager::IoManager(size_t threadNum, bool callerJoin, const std::string& name) :
    Scheduler(threadNum, callerJoin, name),
    epollFd_(::epoll_create1(EPOLL_CLOEXEC)),
    pendingEventCount_{0},
    fdContexts_(),
    mutex_()
{
    if (epollFd_ < 0)
    {
        LOG_FAIL(LOG_ROOT)<<"IoManager::IoManager epoll_create1 fail";
        exit(-1);
    }


    

    if (::pipe(pipefd_) < 0) {
        LOG_FAIL(LOG_ROOT) << "IoManager::IoManager pipe fail";
        exit(-1);
    }

    BASE::SetNoBlock(pipefd_[0]);
    //将唤醒描述符的读写事件回调添加进来
    addEvent(pipefd_[0], READ, std::bind(&IoManager::tickleRead, this));
    --pendingEventCount_;
    //addEvent(ticklefd_, READ, std::bind(&IoManager::tickleRead, this));

    start();
}


IoManager::~IoManager()
{
    close(epollFd_);
    close(pipefd_[0]);
    close(pipefd_[1]);

    for(auto& i : fdContexts_)
    {
        close(i.first);
    }
}

//增加事件
int IoManager::addEvent(int fd, EventType type, std::function<void()> cb)
{
    int op;
    FdContext::ptr con;
    {
        //对IoManager进行加锁
        Mutex::Lock lock(mutex_);

        
        auto it = fdContexts_.find(fd);
        if (it == fdContexts_.end())    //事件第一次被添加进来
        {
            op = EPOLL_CTL_ADD;
            con.reset( new FdContext);
            fdContexts_[fd] = con ;
            fdContexts_[fd]->fd = fd;
            fdContexts_[fd]->event = type;
        }
        else
        {
            con = it->second;
            //相同的事件被添加进来
            if (con->event & type)
            {
                LOG_ERROR(LOG_ROOT)<<"IoManager::addEvent same type ="<<type<<" fd = "<<fd;
            }
            
            op = EPOLL_CTL_MOD;
            con->event = static_cast<EventType> (type | con->event);
        }
    }
    
    //对fd的操作进行加锁
    Mutex::Lock lock(con->fdMutex_);
    
    epoll_event event;
    if (fd == pipefd_[0])
    {
        event.events = con->event;
    }
    else
    {
        event.events = EPOLLET | con->event;
    }
    event.data.ptr = con.get();


    if (::epoll_ctl(epollFd_, op, fd, &event) < 0)
    {
        LOG_ERROR(LOG_ROOT)<<"IoManager::addEvent epoll_ctl  fail";
        return -1;
    }

    ++pendingEventCount_;
    
    FdContext::EventContext& eventCtx = con->getContext(type);
    eventCtx.scheduler = Scheduler::GetCurrentScheduler();
    if (cb)
    {
        eventCtx.cb.swap(cb);
    }
    else    //否则就把当前的协程赋值给eventCtx
    {
        eventCtx.fiber = Fiber::GetCurFiber();
        ZZG_ASSERT(eventCtx.fiber->getState() == Fiber::State::RUNNING);
    }



    return 0;
}


//删除一个fd下面对应的事件
int IoManager::delEvent(int fd, EventType type)
{
    int op;
    FdContext::ptr FdCtx;
    {
        Mutex::Lock lock(mutex_);

        auto it = fdContexts_.find(fd);
        if (it == fdContexts_.end())    //删除一个已经不存在的事件了
        {
            LOG_ERROR(LOG_ROOT)<<"IoManager::delEvent fail fd = "<<fd<<" not exist";
            ZZG_ASSERT(false);
            return -1;
        }
        else
        {
            FdCtx = it->second;
            if (!(FdCtx->event & type))
            {
                LOG_ERROR(LOG_ROOT)<<"IoManager::delEvent  no this event = "<<type;
                ZZG_ASSERT(false);
                return -1;
            }
            

            FdCtx->event = static_cast<EventType> (FdCtx->event & ~type);
            if (FdCtx->event == NONE)//当前已经没有关注的事件了
            {
                op = EPOLL_CTL_DEL;
            }
            else
            {
                op = EPOLL_CTL_MOD;
            }
        }
    }

    {
        Mutex::Lock lock(FdCtx->fdMutex_);

        epoll_event event;
        event.events = EPOLLET | FdCtx->event;
        event.data.ptr = FdCtx.get();
        if (::epoll_ctl(epollFd_, op, fd, &event) < 0)
        {
            LOG_ERROR(LOG_ROOT)<<"IoManager::delEvent epoll_ctl  fail";
            return -1;
        }

        --pendingEventCount_;

        //将事件进行重置
        FdContext::EventContext& eventCtx = FdCtx->getContext(type);
        FdCtx->resetContext(eventCtx);
    }


    if (op == EPOLL_CTL_DEL)
    {
        Mutex::Lock lock(mutex_);

        auto it = fdContexts_.find(fd);
        
        if (it != fdContexts_.end())
        {
            LOG_INFO(LOG_ROOT)<<"IoManager::delEvent  fd = "<<fd;
            fdContexts_.erase(it);
        }
        else
        {
            LOG_INFO(LOG_ROOT)<<"IoManager::delEvent  fd = "<<fd<<" but not find";
        }  
    }

    return 0;
}

int IoManager::cancelEvent(int fd, EventType type)
{
    int op;
    FdContext::ptr FdCtx;

    {
        Mutex::Lock lock(mutex_);

        
        auto it = fdContexts_.find(fd);
        if (it == fdContexts_.end())    //取消一个已经不存在的事件了
        {
            LOG_ERROR(LOG_ROOT)<<"IoManager::cancelEvent fail fd = "<<fd<<" not exist";
            ZZG_ASSERT(false);
            return -1;
        }
        else
        {
            FdCtx = it->second;
            if (!(FdCtx->event & type))
            {
                LOG_ERROR(LOG_ROOT)<<"IoManager::cancelEvent  no this event = "<<type;
                ZZG_ASSERT(false);
                return -1;
            }
            

            FdCtx->event = static_cast<EventType> (FdCtx->event & ~type);
            if (FdCtx->event == NONE)//当前已经没有关注的事件了
            {
                op = EPOLL_CTL_DEL;
            }
            else
            {
                op = EPOLL_CTL_MOD;
            }
        }
    }


    {
        Mutex::Lock lock(FdCtx->fdMutex_);
        epoll_event event;
        event.events = EPOLLET | FdCtx->event;
        event.data.ptr = FdCtx.get();
        if (::epoll_ctl(epollFd_, op, fd, &event) < 0)
        {
            LOG_ERROR(LOG_ROOT)<<"IoManager::cancelEvent epoll_ctl  fail";
            return -1;
        }

        --pendingEventCount_;

        //将事件进行触发
        FdContext::EventContext& eventCtx = FdCtx->getContext(type);
        FdCtx->triggerEvent(type);
    }


    
    if (op == EPOLL_CTL_DEL)
    {
        Mutex::Lock lock(mutex_);

        auto it = fdContexts_.find(fd);
        
        if (it != fdContexts_.end())
        {
            LOG_INFO(LOG_ROOT)<<"IoManager::cancelEvent  fd = "<<fd;
            fdContexts_.erase(it);
        }
        else
        {
            LOG_INFO(LOG_ROOT)<<"IoManager::cancelEvent  fd = "<<fd<<" but not find";
        }  
    }
    
    return 0;
}

int IoManager::cancalAll(int fd)
{
    int op = EPOLL_CTL_DEL;
    FdContext::ptr FdCtx;
    {
        Mutex::Lock lock(mutex_);

        auto it = fdContexts_.find(fd);
        if (it == fdContexts_.end())    //取消一个已经不存在的事件了
        {
            LOG_ERROR(LOG_ROOT)<<"IoManager::cancalAll fail fd = "<<fd<<" not exist";
            ZZG_ASSERT(false);
            return -1;
        }
        else
        {
            FdCtx = it->second;
            fdContexts_.erase(it);
            if ((FdCtx->event == EventType::NONE))
            {
                LOG_ERROR(LOG_ROOT)<<"IoManager::cancalAll  no this event = ";
                ZZG_ASSERT(false);
                return -1;
            }
        }

    }

    Mutex::Lock lock(FdCtx->fdMutex_);
    epoll_event event;
    event.events = 0;
    event.data.ptr = FdCtx.get();
    if (::epoll_ctl(epollFd_, op, fd, &event) < 0)
    {
        LOG_ERROR(LOG_ROOT)<<"IoManager::cancalAll epoll_ctl  fail";
        return -1;
    }


    if (FdCtx->event & EventType::READ)
    {
        FdCtx->triggerEvent(EventType::READ);
        --pendingEventCount_;
    }

    if (FdCtx->event & EventType::WRITE)
    {
        FdCtx->triggerEvent(EventType::WRITE);
        --pendingEventCount_;
    }

    FdCtx->event = EventType::NONE;
    


    LOG_INFO(LOG_ROOT)<<"IoManager::cancalAll  fd = "<<fd;
    return 0;
}


//从阻塞中唤醒
void IoManager::tickle() 
{
    uint8_t signal = 1;
    if (::write(pipefd_[1], &signal, sizeof(signal)) != sizeof(signal)) 
    {
        LOG_ERROR(LOG_ROOT) << "IoManager::tickle ERROR";
    }
}

bool IoManager::isStop()
{
    return Scheduler::isStop() && pendingEventCount_ == 0;    
}


void IoManager::tickleRead() 
{
    uint8_t signal = 0;
    if(::read(pipefd_[0], &signal, sizeof(signal)) > 0) 
    {
        LOG_INFO(LOG_ROOT) << "tickleRead triggered";
        // 唤醒任务或 idle 协程
    }
}

void IoManager::idle()
{
    const int MaxEvents = 256;
    epoll_event * events = new epoll_event[MaxEvents];


    while (true)
    {
        LOG_INFO(LOG_ROOT)<<"IoManager::idle coming";
        if (isStop())
        {
            LOG_INFO(LOG_ROOT)<<"IoManager::idle isStop";
            break;
        }


        int count = 0;
        do
        {
            const int MaxTimeOut = 3000;
            count = epoll_wait(epollFd_, events, MaxEvents, MaxTimeOut);
            //当发生超时的时候 count是0
            if (count < 0 )
            {
                int err = errno;

                if (err == EINTR)
                {
                    continue;
                }
                else
                {
                    LOG_ERROR(LOG_ROOT)<<"IoManager::idle epoll_wait ERROR="<<err;
                    return;
                }
                
            }
            else
            {
                break;
            }
            
        } while (true);

        for (int i = 0; i < count; i++)
        {
            FdContext * fdCtx = static_cast<FdContext*>(events[i].data.ptr);
            
            //操作进行加锁
            Mutex::Lock lock(fdCtx->fdMutex_);

            if (fdCtx->fd == pipefd_[0])
            {
                tickleRead();
                continue;
            }

            int realEvents = EventType::NONE;
            if (events[i].events & EventType::READ)
            {
                realEvents |= EventType::READ;
            }

            if (events[i].events & EventType::WRITE)
            {
                realEvents |= EventType::WRITE;
            }

            int remainEvents = fdCtx->event & ~realEvents;
            int op = remainEvents ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
            epoll_event newEvent;
            newEvent.events = EPOLLET | remainEvents;
            newEvent.data.ptr = fdCtx;
            if (::epoll_ctl(epollFd_, op, fdCtx->fd, &newEvent) < 0)
            {
                LOG_ERROR(LOG_ROOT)<<"IoManager::idle epoll_ctl  fail fd="<<fdCtx->fd;
            }

            if (events[i].events & EventType::READ)
            {
                fdCtx->triggerEvent(EventType::READ);
                --pendingEventCount_;
            }

            if (events[i].events & EventType::WRITE)
            {
                fdCtx->triggerEvent(EventType::WRITE);
                --pendingEventCount_;
            }
        }
        
        
        //从idle中被唤醒了 然后需要去跑run方法了
        Fiber::ptr CurFiber = Fiber::GetCurFiber();
        CurFiber->yield();
    }
    


    delete [] events;
}

}