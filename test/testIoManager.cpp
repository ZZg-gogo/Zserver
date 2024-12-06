#include "../base/Logger.h"
#include "../base/IoManager.h"
#include <time.h>
#include <thread>

void fun()
{
    LOG_INFO(LOG_ROOT)<<"fun fun";
}

int main()
{

    BASE::IoManager Manager{3, false, "IOM"};
    
    sleep(5);
    Manager.stop();
    /*while (true)
    {
        //sleep(1);
        Manager.addJob(fun);
    }*/
    

    return 0;
}