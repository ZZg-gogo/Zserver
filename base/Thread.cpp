#include "Thread.h"
#include "Logger.h"
#include "util.h"

namespace BASE
{

static thread_local Thread* ThreadPtr = nullptr;
static thread_local std::string ThreadName = "MainThread";

Thread* Thread::getCurThread() //获取当前的线程
{
    return ThreadPtr;
}  

std::string& Thread::GetName()  //获取当前的线程名
{
    return ThreadName;
}



void * Thread::run(void * arg)
{
    Thread* thread = static_cast<Thread*>(arg);
    ThreadPtr = thread;
    ThreadName = thread->getName();

    thread->threadPid_ = getThreadId();
    pthread_setname_np(thread->thread_, ThreadName.c_str());

    std::function<void()> cb;
    cb.swap(thread->threadFun_);
    cb();

    //thread->thread_ = 0;
}

Thread::Thread(ThreadCallback cb, const std::string& name) :
    threadFun_(cb),
    threadName_(name),
    thread_(0),
    threadPid_(0)
{
    int ret = pthread_create(&thread_, nullptr, run, this);
    if (ret)
    {
        LOG_INFO(LOG_ROOT)<<"pthread_create error name = "<<name;
        throw std::logic_error("pthread_create");
    }
    
}


Thread::~Thread()
{
    if (thread_)
    {
        pthread_detach(thread_);
    }
    
}

void Thread::join()
{
    if (thread_)
    {
        pthread_join(thread_, nullptr);
        thread_ = 0;
    }
}

}