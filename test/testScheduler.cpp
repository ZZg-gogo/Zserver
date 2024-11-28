#include "../base/Logger.h"
#include "../base/Scheduler.h"


void fun(void)
{
    LOG_INFO(LOG_ROOT)<<"my fun ...... ";
}

int main(int argc, char **argv)
{
    BASE::Scheduler scheduler(3, true);

    scheduler.addJob(fun);

    scheduler.start();

    LOG_INFO(LOG_ROOT)<<"begin sleep";

    sleep(10);

    return 0;
}