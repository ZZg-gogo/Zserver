#pragma once

#include <pthread.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

namespace BASE
{

pid_t getThreadId();    //获取线程id
u_int32_t getFiberId(); //获取携程id








}