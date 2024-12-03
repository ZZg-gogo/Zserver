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
    ticklefd_(::eventfd(0, EFD_CLOEXEC|EFD_NONBLOCK)),
    pendingEventCount_{0},
    fdContexts_(),
    mutex_()
{
    if (epollFd_ < 0)
    {
        LOG_FAIL(LOG_ROOT)<<"IoManager::IoManager epoll_create1 fail";
        exit(-1);
    }

    if (ticklefd_ < 0)
    {
        LOG_FAIL(LOG_ROOT)<<"IoManager::IoManager eventfd fail";
        exit(-1);
    }
    
    //将唤醒描述符的读写事件回调添加进来
    addEvent(ticklefd_, WRITE, std::bind(&IoManager::tickle, this));
    addEvent(ticklefd_, READ, std::bind(&IoManager::tickleRead, this));

    start();
}


IoManager::~IoManager()
{
    close(epollFd_);
    close(ticklefd_);

    for(auto& i : fdContexts_)
    {
        close(i.first);
    }
}

//增加事件
int IoManager::addEvent(int fd, EventType type, std::function<void()> cb)
{
    Mutex::Lock lock(mutex_);

    int op;
    auto it = fdContexts_.find(fd);
    if (it == fdContexts_.end())    //事件第一次被添加进来
    {
        op = EPOLL_CTL_ADD;
        FdContext::ptr con { new FdContext};
        fdContexts_[fd] = con ;
        fdContexts_[fd]->fd = fd;
        fdContexts_[fd]->event = type;
    }
    else
    {
        //相同的事件被添加进来
        if (fdContexts_[fd]->event & type)
        {
            LOG_ERROR(LOG_ROOT)<<"IoManager::addEvent same type ="<<type<<" fd = "<<fd;
        }
        

        op = EPOLL_CTL_MOD;
        fdContexts_[fd]->event = static_cast<EventType> (type | fdContexts_[fd]->event);
    }
    
    epoll_event event;
    event.events = EPOLLET | fdContexts_[fd]->event;
    event.data.ptr = fdContexts_[fd].get();

    if (::epoll_ctl(epollFd_, op, fd, &event) < 0)
    {
        LOG_ERROR(LOG_ROOT)<<"IoManager::addEvent epoll_ctl  fail";
        return -1;
    }

    ++pendingEventCount_;
    
    FdContext::EventContext& eventCtx = fdContexts_[fd]->getContext(type);
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
    Mutex::Lock lock(mutex_);

    int op;
    auto it = fdContexts_.find(fd);
    if (it == fdContexts_.end())    //删除一个已经不存在的事件了
    {
        LOG_ERROR(LOG_ROOT)<<"IoManager::delEvent fail fd = "<<fd<<" not exist";
        ZZG_ASSERT(false);
        return -1;
    }
    else
    {
        if (!(fdContexts_[fd]->event & type))
        {
            LOG_ERROR(LOG_ROOT)<<"IoManager::delEvent  no this event = "<<type;
            ZZG_ASSERT(false);
            return -1;
        }
        

        fdContexts_[fd]->event = static_cast<EventType> (fdContexts_[fd]->event & ~type);
        if (fdContexts_[fd]->event == NONE)//当前已经没有关注的事件了
        {
            op = EPOLL_CTL_DEL;
        }
        else
        {
            op = EPOLL_CTL_MOD;
        }
    }

    epoll_event event;
    event.events = EPOLLET | fdContexts_[fd]->event;
    event.data.ptr = fdContexts_[fd].get();
    if (::epoll_ctl(epollFd_, op, fd, &event) < 0)
    {
        LOG_ERROR(LOG_ROOT)<<"IoManager::delEvent epoll_ctl  fail";
        return -1;
    }

    --pendingEventCount_;

    //将事件进行重置
    FdContext::EventContext& eventCtx = fdContexts_[fd]->getContext(type);
    fdContexts_[fd]->resetContext(eventCtx);

    if (op == EPOLL_CTL_DEL)
    {
        LOG_INFO(LOG_ROOT)<<"IoManager::delEvent  fd = "<<fd;
        fdContexts_.erase(it);
    }

    return 0;
}

int IoManager::cancelEvent(int fd, EventType type)
{
    Mutex::Lock lock(mutex_);

    int op;
    auto it = fdContexts_.find(fd);
    if (it == fdContexts_.end())    //取消一个已经不存在的事件了
    {
        LOG_ERROR(LOG_ROOT)<<"IoManager::cancelEvent fail fd = "<<fd<<" not exist";
        ZZG_ASSERT(false);
        return -1;
    }
    else
    {
        if (!(fdContexts_[fd]->event & type))
        {
            LOG_ERROR(LOG_ROOT)<<"IoManager::cancelEvent  no this event = "<<type;
            ZZG_ASSERT(false);
            return -1;
        }
        

        fdContexts_[fd]->event = static_cast<EventType> (fdContexts_[fd]->event & ~type);
        if (fdContexts_[fd]->event == NONE)//当前已经没有关注的事件了
        {
            op = EPOLL_CTL_DEL;
        }
        else
        {
            op = EPOLL_CTL_MOD;
        }
    }

    epoll_event event;
    event.events = EPOLLET | fdContexts_[fd]->event;
    event.data.ptr = fdContexts_[fd].get();
    if (::epoll_ctl(epollFd_, op, fd, &event) < 0)
    {
        LOG_ERROR(LOG_ROOT)<<"IoManager::cancelEvent epoll_ctl  fail";
        return -1;
    }

    --pendingEventCount_;

    //将事件进行触发
    FdContext::EventContext& eventCtx = fdContexts_[fd]->getContext(type);
    fdContexts_[fd]->triggerEvent(type);
    

    if (op == EPOLL_CTL_DEL)
    {
        LOG_INFO(LOG_ROOT)<<"IoManager::cancelEvent  fd = "<<fd;
        fdContexts_.erase(it);
    }
    
    //fdContexts_[fd]->resetContext(eventCtx);

    return 0;
}

int IoManager::cancalAll(int fd)
{

    Mutex::Lock lock(mutex_);

    int op = EPOLL_CTL_DEL;
    auto it = fdContexts_.find(fd);
    if (it == fdContexts_.end())    //取消一个已经不存在的事件了
    {
        LOG_ERROR(LOG_ROOT)<<"IoManager::cancalAll fail fd = "<<fd<<" not exist";
        ZZG_ASSERT(false);
        return -1;
    }
    else
    {
        if ((fdContexts_[fd]->event == EventType::NONE))
        {
            LOG_ERROR(LOG_ROOT)<<"IoManager::cancalAll  no this event = ";
            ZZG_ASSERT(false);
            return -1;
        }
    }

    epoll_event event;
    event.events = 0;
    event.data.ptr = fdContexts_[fd].get();
    if (::epoll_ctl(epollFd_, op, fd, &event) < 0)
    {
        LOG_ERROR(LOG_ROOT)<<"IoManager::cancalAll epoll_ctl  fail";
        return -1;
    }


    if (fdContexts_[fd]->event & EventType::READ)
    {
        fdContexts_[fd]->triggerEvent(EventType::READ);
        --pendingEventCount_;
    }

    if (fdContexts_[fd]->event & EventType::WRITE)
    {
        fdContexts_[fd]->triggerEvent(EventType::WRITE);
        --pendingEventCount_;
    }

    fdContexts_[fd]->event = EventType::NONE;
    
    fdContexts_.erase(it);

    LOG_INFO(LOG_ROOT)<<"IoManager::cancalAll  fd = "<<fd;
    return 0;
}


//从阻塞中唤醒
void IoManager::tickle() 
{
    uint64_t num = 1;
    if (sizeof(num) != ::write(ticklefd_, &num, sizeof(num)))
    {
        LOG_ERROR(LOG_ROOT)<<"IoManager::tickle ERROR";
    }
}

void IoManager::tickleRead()
{
    uint64_t num = 0;
    if (sizeof(num) != ::read(ticklefd_, &num, sizeof(num)))
    {
        LOG_ERROR(LOG_ROOT)<<"IoManager::tickleRead ERROR";
    }
}

}