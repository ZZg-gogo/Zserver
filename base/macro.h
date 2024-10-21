#pragma once

#include "Logger.h"
#include "util.h"
#include <assert.h>

#define ZZG_ASSERT(expr) { \
    if (!(expr))\
    {\
        LOG_ERROR(LOG_ROOT)<<"Assert Error:" #expr<<"\n"<<BASE::Backtrace(15,0);\
        assert(expr);\
    }\
}