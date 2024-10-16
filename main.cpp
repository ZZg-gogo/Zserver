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


void fun()
{
    std::cout<<"666666666"<<std::endl;
    sleep(20);
}

int main(int argc, char ** argv)
{
    std::vector<BASE::Thread::ptr> threads;
    for (int i = 0; i < 5; i++)
    {
        threads.push_back(std::make_shared<BASE::Thread>(fun, "ZZH" + std::to_string(i)));
    }
    
    for (int i = 0; i < 5; i++)
    {
        threads[i]->join();
    }

    return 0;
}