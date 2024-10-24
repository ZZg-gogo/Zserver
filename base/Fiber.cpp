#include "Fiber.h"
#include "Config.h"


namespace BASE
{

static thread_local Fiber* MainFiber = nullptr;
static thread_local Fiber::ptr CurFiber = nullptr;

std::atomic_int64_t Fiber::FibersCount = 0;
std::atomic_int64_t Fiber::FiberId = 0;

static ConfigVar<uint32_t>::ptr StackSizeCfg  =  ConfigVar<uint32_t>::Create("Fiber.StackSize", "协程栈大小", 1024*1024);


Fiber::Fiber(FiberFun fun, size_t stackSize) : 
    fiberId_(nextFiberId()),    
    stackSize_(stackSize),
    state_(State::INIT),
    ucontext_(),
    stackPoint_(nullptr),
    cb_(fun)
{

    
}

Fiber::~Fiber()
{

}

}//end namespace