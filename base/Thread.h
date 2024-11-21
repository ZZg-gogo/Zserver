#pragma once

#include <pthread.h>
#include <thread>
#include <memory>
#include <string>
#include <functional>
#include "Lock.h"
#include "NoCopyable.h"


namespace BASE
{

class Thread : NoCopyable
{
public:
    typedef std::shared_ptr<Thread> ptr;
    typedef std::function<void()> ThreadCallback;

public:
    Thread(ThreadCallback cb, const std::string& name);
    ~Thread();

    void join();
    pid_t getId() const {return threadPid_;}
    std::string& getName() {return threadName_;}

    static Thread* getCurThread();  //获取当前的线程
    static std::string& GetName();  //获取当前的线程名

    static void * run(void * arg);  //传递给POSIX线程参数
private:
    pid_t threadPid_;    //线程id
    pthread_t thread_;  //线程id
    std::string threadName_;    //线程名 用thread_local
    ThreadCallback threadFun_;  //线程函数
    Semaphore sem_;     //信号量
};


}//end namespace





