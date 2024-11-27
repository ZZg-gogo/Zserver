#include "../base/Logger.h"
#include "../base/Scheduler.h"


int main(int argc, char **argv)
{
    BASE::Scheduler scheduler(1, false);

    scheduler.start();

    LOG_INFO(LOG_ROOT)<<"begin sleep";

    sleep(10);

    return 0;
}