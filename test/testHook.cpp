#include "../base/Hook.h"
#include "../base/Logger.h"
#include "../base/IoManager.h"
#include <time.h>
#include <thread>
#include <atomic>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>



int main(int argc, char ** argv)
{
    BASE::IoManager man(1);

    man.addJob([](){
        sleep(2);
        LOG_INFO(LOG_ROOT)<<"sleep 2";
    });

    man.addJob([](){
        sleep(3);
        LOG_INFO(LOG_ROOT)<<"sleep 3";
    });

    LOG_INFO(LOG_ROOT)<<"main end";

    return 0;
}