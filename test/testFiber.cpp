#include "../base/Fiber.h"
#include "../base/Logger.h"


void fun()
{
    for (int  i = 0; i < 10000; i++)
    {
        if (i % 5 == 0)
        {
            BASE::Fiber::ptr cur = BASE::Fiber::GetCurFiber();
            BASE::Fiber::YieldToSuspended();   //让出执行权
        }
        LOG_INFO(LOG_ROOT)<<"i = "<<i;
        sleep(1);
    }
    
}


int main()
{
     //获取一下当前协程 main
    BASE::Fiber::ptr cur = BASE::Fiber::GetCurFiber();

    BASE::Fiber::ptr co {new BASE::Fiber{fun, 0}};
    LOG_INFO(LOG_ROOT)<<"main start ";
    co->resume();    //开始执行了
    LOG_INFO(LOG_ROOT)<<"yield ";
    co->resume();    //开始执行了
    LOG_INFO(LOG_ROOT)<<"yield ";
    co->resume();    //开始执行了
    LOG_INFO(LOG_ROOT)<<"yield ";
    co->resume();    //开始执行了
    LOG_INFO(LOG_ROOT)<<"yield ";
    co->resume();    //开始执行了
    LOG_INFO(LOG_ROOT)<<"yield ";
    return 0;
}