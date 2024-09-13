#pragma once

#include <time.h>
#include <string>
#include <memory>
#include <list>
#include <fstream>

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
    virtual void log(LoggerLevel level, LoggerContent::ptr content) = 0;
    virtual ~LoggerAppend() {}
    //设置日志的输出格式
    void setFormat(LoggerFormat::ptr format) {format_ = format;}
    LoggerFormat::ptr getFormat() {return format_;}
protected:
    LoggerFormat::ptr format_;
};

class Logger    //日志器
{
public:
    typedef std::shared_ptr<Logger> ptr;

public:
    Logger(const std::string &name = "Zlog");

    void log(LoggerLevel level, LoggerContent::ptr content);
    void debug(LoggerContent::ptr content);
    void info(LoggerContent::ptr content);
    void warn(LoggerContent::ptr content);
    void error(LoggerContent::ptr content);
    void fail(LoggerContent::ptr content);

    void addAppend(LoggerAppend::ptr append);
    void delAppend(LoggerAppend::ptr append);

    //设置获取等级
    LoggerLevel getLevel() const {return level_;}
    void setLevel(LoggerLevel level) {level_ = level;}
private:
    std::string name_;
    LoggerLevel level_; //日志级别 日志级别大于level_才会被输出
    std::list<LoggerAppend::ptr> appenders_;    //输出的目的地集合 
     
};


class StdoutLogAppend : public LoggerAppend //输出到控制台
{
public:
    typedef std::shared_ptr<StdoutLogAppend> ptr;

public:
    virtual void log(LoggerLevel level, LoggerContent::ptr content) override;

private:
};

class FileLogAppend : public LoggerAppend   //输出到文件
{
public:
    typedef std::shared_ptr<FileLogAppend> ptr;

public:
    FileLogAppend(const std::string & filename);    //输出到哪个文件
    virtual void log(LoggerLevel level, LoggerContent::ptr content) override;
    void reopen();  //需要打开文件
private:
    std::string filename_;  //文件名
    std::ofstream filestream_;
};

}