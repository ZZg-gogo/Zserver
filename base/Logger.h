#pragma once

#include <time.h>
#include <string>
#include <memory>

namespace BASE
{

class LoggerContent //日志内容
{
public:
    typedef std::shared_ptr<LoggerContent> ptr;
    LoggerContent(std::string &content);
private:
    const char * file_; //文件名
    int line_;  //行号
    int threadId_;  //线程id
    int fiberId_;   //携程id
    time_t time_;   //时间戳
    std::string content_;   //内容
};


enum class LoggerLevel  //日志级别
{
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FAIL = 5
};


class LoggerFormat  //日志格式化
{
public:
    typedef std::shared_ptr<LoggerFormat> ptr;
public:
    std::string format(LoggerContent::ptr content);
private:
};


class LoggerAppend  //日志输出
{
public:
    typedef std::shared_ptr<LoggerAppend> ptr;

public:
    void log(LoggerLevel level, LoggerContent::ptr content);
    virtual ~LoggerAppend() {}
private:

};

class Logger    //日志器
{
public:
    typedef std::shared_ptr<Logger> ptr;

public:
    Logger(const std::string &name = "Zlog");

    void log(LoggerLevel level, LoggerContent::ptr content);
private:
    std::string name_;
    LoggerLevel level_;
    LoggerAppend::ptr appender_;
};


class StdoutLogAppend : public LoggerAppend //输出到控制台
{

};

class FileLogAppend : public LoggerAppend   //输出到文件
{

};

}