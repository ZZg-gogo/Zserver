#include "../base/Fiber.h"
#include "../base/Logger.h"
#include "../base/Thread.h"

void fun()
{
    for (int  i = 0; i < 20; i++)
    {
        if (i % 5 == 0)
        {
            BASE::Fiber::ptr cur = BASE::Fiber::GetCurFiber();
            BASE::Fiber::YieldToSuspended();   //让出执行权
        }
        LOG_INFO(LOG_ROOT)<<"i = "<<i;
    }
    BASE::Fiber::YieldToSuspended();   //让出执行权

    LOG_INFO(LOG_ROOT)<<"fun end";
}


void test()
{

    //获取一下当前协程 main
    BASE::Fiber::ptr cur = BASE::Fiber::GetCurFiber();

    BASE::Fiber::ptr co {new BASE::Fiber{fun, 0}};
    LOG_INFO(LOG_ROOT)<<"main start "<<co.use_count();
    co->resume();    //开始执行了
    LOG_INFO(LOG_ROOT)<<"yield ";
    co->resume();    //开始执行了
    LOG_INFO(LOG_ROOT)<<"yield ";
    co->resume();    //开始执行了
    LOG_INFO(LOG_ROOT)<<"yield ";
    co->resume();    //开始执行了
    LOG_INFO(LOG_ROOT)<<"yield ";
    co->resume();    //开始执行了
    LOG_INFO(LOG_ROOT)<<"yield "<<co.use_count();
    co->resume();    //开始执行了
}

int main()
{
    std::vector<BASE::Thread::ptr> threads;

    for (int i = 0; i < 3; i++)
    {
        threads.push_back(BASE::Thread::ptr (new BASE::Thread(test, "Zthread" + std::to_string(i))));
    }

    for (int i = 0; i < 3; i++)
    {
        threads[i]->join();
    }
    

    

    return 0;
}