#include <iostream>
#include <time.h>
#include <boost/lexical_cast.hpp>

#include "base/Logger.h"
#include "base/Singleton.h"
#include "base/util.h"
#include "base/Config.h"

BASE::ConfigVar<int>::ptr g_int(new BASE::ConfigVar<int>("prot", "listenPort", 8080));

BASE::Config::ptr config;

int main(int argc, char ** argv)
{
    LOG_INFO(LOG_ROOT)<<g_int->toString();
    LOG_INFO(LOG_ROOT)<<g_int->parseFromString(g_int->toString());
    config->create<BASE::ConfigVar<int>>(g_int);
    config->create<BASE::ConfigVar<int>>(g_int);
    config->lookup<BASE::ConfigVar<int>>("port")->toString();
    return 0;
}