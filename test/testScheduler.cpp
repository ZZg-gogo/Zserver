#include "../base/Logger.h"
#include "../base/Scheduler.h"


void fun(void)
{
    LOG_INFO(LOG_ROOT)<<"my fun ...... ";
}

int main(int argc, char **argv)
{
    BASE::Scheduler scheduler(3, true);

    for (int i = 0; i < 100; i++)
    {
        scheduler.addJob(fun, 1);
    }
    
    

    scheduler.start();

    LOG_INFO(LOG_ROOT)<<"main end";

    //sleep(10);

    return 0;
}