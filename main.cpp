#include <iostream>
#include <time.h>
#include <unistd.h>
#include <boost/lexical_cast.hpp>

#include <yaml-cpp/yaml.h>
#include <yaml-cpp/node/parse.h>

#include "base/Logger.h"
#include "base/Singleton.h"
#include "base/util.h"
#include "base/Config.h"
#include "base/Thread.h"
#include "base/Lock.h"


int count = 0;
BASE::RWmutex mutex;

void fun()
{
    for (int i = 0; i < 1000000; i++)
    {
        BASE::RWmutex::rLock lock(mutex);
        ++count;
    }
    
}

int main(int argc, char ** argv)
{
    LOG_INFO(LOG_ROOT)<<"ZZG Server Start";
    std::vector<BASE::Thread::ptr> threads;
    for (int i = 0; i < 5; i++)
    {
        threads.push_back(std::make_shared<BASE::Thread>(fun, "ZZH" + std::to_string(i)));
    }
    
    for (int i = 0; i < 5; i++)
    {
        threads[i]->join();
    }

    std::cout<<count<<std::endl;

    LOG_INFO(LOG_ROOT)<<"ZZG Server Quit";
    return 0;
}