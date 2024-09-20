#include <iostream>
#include <time.h>

#include "base/Logger.h"
#include "base/Singleton.h"
#include "base/util.h"

int main(int argc, char ** argv)
{
    BASE::Logger::ptr logger(new BASE::Logger()); 
    
    logger->addAppend(BASE::LoggerAppend::ptr(new BASE::StdoutLogAppend()));

    BASE::LoggerContent::ptr content(new BASE::LoggerContent(logger, BASE::LoggerLevel::INFO, __FILE__,
        __LINE__, BASE::getThreadId(), BASE::getFiberId(), time(nullptr), "Mythread"));
    content->format("Hello World %s", "ZZH");
    BASE::LoggerContentWrap wrap(content);

    LOG_FORMAT_LOGGER(logger, BASE::LoggerLevel::WARN, "123456 %s", "ZZZ");
    LOG_FORMAT_INFO(logger, "789 %s", "HHH");

    LOG_STREAM_LOGGER(logger, BASE::LoggerLevel::WARN)<<"我是你*";

    int a = 10;
    LOG_ERROR(logger)<<"你好啊"<<a;
    return 0;
}