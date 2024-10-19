#include <iostream>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <execinfo.h>

#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/node/parse.h>

#include "../base/Logger.h"
#include "../base/Singleton.h"
#include "../base/util.h"
#include "../base/Config.h"
#include "../base/Thread.h"
#include "../base/Lock.h"


void testAssert()
{
    
    LOG_INFO(LOG_ROOT)<<BASE::Backtrace(11, 0); 

    std::vector<std::string> v;
    BASE::Backtrace(v, 10, 1);
    for(auto&i:v)
    {
        LOG_INFO(LOG_ROOT)<<i;
    }
    assert(0);
}

int main(int argc, char ** argv)
{
    testAssert();
    std::cout<<"test util"<<std::endl;
    return 0;
}