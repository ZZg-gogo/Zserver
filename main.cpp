#include <iostream>
#include <time.h>
#include "base/Logger.h"


int main(int argc, char ** argv)
{
    BASE::Logger::ptr logger(new BASE::Logger());
    logger->addAppend(BASE::LoggerAppend::ptr(new BASE::StdoutLogAppend()));

    BASE::LoggerContent::ptr content(new BASE::LoggerContent(logger, BASE::LoggerLevel::INFO, __FILE__,
        __LINE__, 888, 777, time(nullptr), "Mythread"));
    content->format("Hello World %s", "ZZH");
    logger->log(content->getLoggerLevel(), content);
    return 0;
}