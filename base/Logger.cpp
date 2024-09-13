#include "Logger.h"

#include <iostream>

namespace BASE
{


Logger::Logger(const std::string &name) :
    name_(name),
    level_(LoggerLevel::INFO)
{
    
}

void Logger::addAppend(LoggerAppend::ptr append)
{
    appenders_.push_back(append);
}

void Logger::delAppend(LoggerAppend::ptr append)
{
    for (auto it = appenders_.begin(); appenders_.end() != it; ++it)
    {
        if (*it == append)
        {
            appenders_.erase(it);
            break;
        }
        
    }
    
}

void Logger::log(LoggerLevel level, LoggerContent::ptr content)
{
    if (level < level_) //日志级别低的不输出
    {
        return;
    }
    
    for (auto it = appenders_.begin(); appenders_.end() != it; ++it)
    {
        (*it)->log(level, content);
    }
    
}
void Logger::debug(LoggerContent::ptr content)
{
    log(LoggerLevel::DEBUG, content);
}
void Logger::info(LoggerContent::ptr content)
{
    log(LoggerLevel::INFO, content);
}
void Logger::warn(LoggerContent::ptr content)
{
    log(LoggerLevel::WARN, content);
}
void Logger::error(LoggerContent::ptr content)
{
    log(LoggerLevel::ERROR, content);
}

void Logger::fail(LoggerContent::ptr content)
{
    log(LoggerLevel::FAIL, content);
}


FileLogAppend::FileLogAppend(const std::string & filename) : 
    filename_(filename)
{

}

void FileLogAppend::log(LoggerLevel level, LoggerContent::ptr content)
{
    filestream_<<format_->format(content);
}

void FileLogAppend::reopen()
{
    if (filestream_)
    {
        filestream_.close();
    }
    
    filestream_.open(filename_);
}

void StdoutLogAppend::log(LoggerLevel level, LoggerContent::ptr content)
{
    std::cout<<format_->format(content);
}








}