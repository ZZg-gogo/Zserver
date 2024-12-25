#include "Hook.h"

#include <iostream>
#include <dlfcn.h>
#include "IoManager.h"


//目前先实现sleep函数
#define HOOK_FUN(XX) \
    XX(sleep)

namespace BASE
{
static thread_local bool hook = false;

bool IsHook()
{
    return hook;
}

void SetHook(bool flag)
{
    hook = flag;
}

}

//在进入到main函数之前 调用该函数初始化hook
struct HookIniter
{
    HookIniter()
    {
        static bool init = false;
        if (init)
        {
            return;
        }
        init = true;
        
        //函数指针 用来保存系统原始的函数地址
        #define XX(name) name ## _fun = (name ## Fun)::dlsym(RTLD_NEXT, #name);
            HOOK_FUN(XX);
        #undef XX
     }
};

static HookIniter initer;


extern "C"
{

//函数指针 用来保存系统原始的函数地址
#define XX(name) name ## Fun  name ## _fun = nullptr;
    HOOK_FUN(XX);
#undef XX


unsigned int sleep(unsigned int seconds)
{
    if (!BASE::hook)
    {
        return ::sleep_fun(seconds);
    }


    BASE::Fiber::ptr fiber = BASE::Fiber::GetCurFiber();
    std::cout<<"66666666666 = "<<fiber<<std::endl;
    BASE::IoManager* scheduler = BASE::IoManager::GetThis();
    scheduler->addTimer(seconds*1000, 
        std::bind(static_cast<void (BASE::IoManager::*)(BASE::Fiber::ptr, int)>(&BASE::IoManager::addJob),
         scheduler, fiber, -1));
        /*scheduler->addTimer(seconds*1000, 
        [](){
            std::cout<<"**********************"<<std::endl;
        });*/
    
    
    BASE::Fiber::YieldToSuspended();
    return 0;
}
}