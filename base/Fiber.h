#pragma once

#include <memory>
#include <ucontext.h>


#include "Lock.h"

namespace BASE
{


class Fiber : public std::enable_shared_from_this<Fiber>
{
public:
    typedef std::shared_ptr<Fiber> ptr;
public:
    Fiber();
    ~Fiber();

private:
    
};







}//end namespace