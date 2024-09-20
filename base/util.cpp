#include "util.h"


namespace BASE
{

pid_t getThreadId()   //获取线程id
{
    return syscall(SYS_gettid);
}

u_int32_t getFiberId()  //获取携程id
{
    return 0;
}




}