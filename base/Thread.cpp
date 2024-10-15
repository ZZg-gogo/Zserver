#include "Thread.h"


namespace BASE
{

static thread_local Thread::ptr ThreadPtr = nullptr;
static thread_local std::string ThreadName = "UNKONW";

Thread::ptr Thread::getCurThread() //获取当前的线程
{
    return ThreadPtr;
}  

std::string& Thread::getName()  //获取当前的线程名
{
    return ThreadName;
}



Thread::Thread(ThreadCallback cb, const std::string& name) :
    threadFun_(cb)
{
    ThreadName = name;
}

}