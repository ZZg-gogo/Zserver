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



std::atomic_int64_t nNum = 0;

void fun()
{
    LOG_INFO(LOG_ROOT)<<"fun fun"<<++nNum;
}

int main()
{

    BASE::IoManager Manager{3, true, "IOM"};
    

    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sockfd == -1) {
        std::cerr << "sockfd failed: " << strerror(errno) << std::endl;
        return -1;
    }

    BASE::SetNoBlock(sockfd);

    Manager.addEvent(sockfd, BASE::IoManager::EventType::WRITE, [](){
        LOG_INFO(LOG_ROOT)<<"connect success"<<++nNum;
    });

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80);  
    server_addr.sin_addr.s_addr = inet_addr("183.2.172.42"); 


    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1 && errno != EINPROGRESS) {
            std::cerr << "connect failed: " << strerror(errno) << std::endl;
        return -1;
    }
    //sleep(5);
    //Manager.stop();
    /*int i = 100;
    while (--i)
    {
        //sleep(1);
        Manager.addJob(fun);
    }*/
    

    return 0;
}