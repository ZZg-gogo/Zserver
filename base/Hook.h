#pragma once

namespace BASE
{

//是否被hook了
extern bool IsHook();
//设置hook
extern void SetHook(bool flag);
}

extern "C"
{
    typedef unsigned int (*sleepFun)(unsigned int seconds);
    extern sleepFun sleep_fun; 
}