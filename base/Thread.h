#pragma once

#include <pthread.h>
#include <thread>
#include <memory>
#include <string>
#include <functional>
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

    static ptr getCurThread();  //获取当前的线程
    static std::string& getName();  //获取当前的线程名
private:
    pid_t threadPid_;    //线程id
    pthread_t thread_;  //线程id
    //std::string threadName_;    //线程名 用thread_local
    ThreadCallback threadFun_;  //线程函数
};


}//end namespace





