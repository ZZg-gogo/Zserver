#pragma once

#include <time.h>
#include <string>
#include <memory>
#include <list>
#include <fstream>
#include <vector>
#include <sstream>
#include <mutex>


namespace BASE
{

enum class LoggerLevel  //日志级别
{
    UNKONW = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FAIL = 5
};

class Logger;

class LoggerContent //日志内容
{
public:
    typedef std::shared_ptr<LoggerContent> ptr;
    LoggerContent(Logger::ptr logger, LoggerLevel level, const char * fileName,
        int line, int threadId, int fiberId, time_t t, const std::string& threadName);

    Logger::ptr getLogger() const {return logger_;}
    LoggerLevel getLoggerLevel() const {return level_;}
    const char * getFileName() const {return file_;}
    int getLine() const {return line_;}
    int getThreadId() const {return threadId_;}
    int getFiberId() const {return fiberId_;}
    int getTime() const {return time_;}
    const std::string& getThreadName() const {return threadName_;}
    std::string getContent() const {return content_.str();}
    std::stringstream& getSS() { return content_;}

    void format(const char* fmt, ...);
    void format(const char* fmt, va_list al);

private:
    Logger::ptr logger_;    //日志器
    LoggerLevel level_; //日志等级
    const char * file_; //文件名
    int line_;  //行号
    int threadId_;  //线程id
    int fiberId_;   //携程id
    time_t time_;   //时间戳
    std::string threadName_;//线程名
    std::stringstream content_;   //内容
};





class LoggerFormat  //日志格式化
{
public:
    typedef std::shared_ptr<LoggerFormat> ptr;

    class FormatItem    //不同的子类输出不同的字段
    {
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        virtual ~FormatItem(){}
        virtual void format(std::ostream& os, LoggerContent::ptr content) = 0;
    };
    
public:
    LoggerFormat(const std::string& pattern);

    std::string format(LoggerContent::ptr content); //日志内容经过格式化输出

private:
    void init();    //去做用户输入的pattern的解析工作
private:
    std::string pattern_;    //存储用户输入的原始格式
    std::vector<FormatItem::ptr> formats_;  //用户指定了哪些输出格式都解析到这个vector里面
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
    Logger(const std::string &name = "Zlog");   //日志器的名字

    void log(LoggerLevel level, LoggerContent::ptr content);
    void debug(LoggerContent::ptr content);
    void info(LoggerContent::ptr content);
    void warn(LoggerContent::ptr content);
    void error(LoggerContent::ptr content);
    void fail(LoggerContent::ptr content);

    //增加/删除日志的输出地
    void addAppend(LoggerAppend::ptr append);
    void delAppend(LoggerAppend::ptr append);

    //设置获取等级
    LoggerLevel getLevel() const {return level_;}
    void setLevel(LoggerLevel level) {level_ = level;}

    //返回日志器的名字
    const std::string& getLoggerName() const {return name_;}

    //设置日志格式器
    void setFormatter(const std::string& val);
    void setFormatter(LogFormatter::ptr formater);
    LoggerFormat::ptr getFormatter();
private:
    std::string name_;
    LoggerLevel level_; //日志级别 日志级别大于level_才会被输出
    std::list<LoggerAppend::ptr> appenders_;    //输出的目的地集合 日志可以被输出到多个地方
    std::mutex mutex_;  //互斥锁
    LoggerFormat::ptr formater_;    //日志格式器
     
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

}//end namespace