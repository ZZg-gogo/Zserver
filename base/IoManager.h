#pragma once

#include "Scheduler.h"
#include <map>
#include <sys/eventfd.h>

namespace BASE
{

class IoManager : public Scheduler
{
public:
    typedef std::shared_ptr<IoManager> ptr;

    //根据epoll.h中定义的数值来的
    enum EventType
    {
        NONE = 0x000,
        READ = 0x001,
        WRITE = 0x004
    };

    //一个文件描述符的运行环境
    struct FdContext
    {
        typedef std::shared_ptr<FdContext> ptr;
        //我这个事件 要放到哪一个协程调度器里面去执行
        struct EventContext
        {
            Scheduler * scheduler;
            std::function <void()> cb;
            Fiber::ptr fiber;
        };

        FdContext() :
            readCallback(),
            writeCallback(),
            event(EventType::NONE),
            fd(-1),
            fdMutex_()
        {}
        //根据事件类型获取到不同事件
        EventContext& getContext(EventType event);
        //将某个事件重置
        void resetContext(EventContext& ctx);
        //触发某个事件
        void triggerEvent(EventType type);

        EventContext readCallback;      //读回调
        EventContext writeCallback;     //写回调
        EventType event;                //我关心的事件类型
        int fd;                         //关联的文件描述符 
        Mutex fdMutex_;                 //多线程操作加锁
    };

public:
    IoManager(size_t threadNum = 1, bool callerJoin = true, const std::string& name = "");
    virtual ~IoManager();

    //增加事件
    int addEvent(int fd, EventType type, std::function<void()> cb = nullptr);
    //删除事件
    int delEvent(int fd, EventType type);
    //取消事件 但是会强制触发事件一次
    int cancelEvent(int fd, EventType type);
    //把该句柄下面的所有事件都取消 事件全都触发
    int cancalAll(int fd);
protected:
    virtual void idle() override;
    virtual void tickle() override;
    //是否已经完全退出了
    virtual bool isStop() override;


public:
    //获取当前的IoManager
    static IoManager * GetThis();

private:
    void tickleRead();

private:
    int epollFd_;                                   //epoll描述符
    int pipefd_[2];                                 //管道
    std::atomic_uint64_t pendingEventCount_;        //当前等待执行的事件数量
    std::map<int , FdContext::ptr> fdContexts_;     //保存所有的fd-->FdContext
    Mutex mutex_;                                   //互斥锁
};





}