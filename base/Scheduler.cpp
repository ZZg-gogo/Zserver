#include "Scheduler.h"


namespace BASE
{


//获取当前正在运行的携程调度器
Scheduler::ptr Scheduler::GetCurrentScheduler()
{
    return nullptr;
}
//获取主协程
Fiber::ptr Scheduler::GetMainFiber()
{
    return nullptr;
}







Scheduler::Scheduler(size_t threadNum, bool callerJoin, const std::string& name) :
    threadNum_(threadNum),
    schedulerName_(name)
{

}


void Scheduler::start()
{

}

void Scheduler::stop()
{

}



}