#include "Logger.h"

#include <sstream>
#include <iostream>
#include <tuple>
#include <cctype>
#include <map>
#include <functional>

namespace BASE
{

struct LoggerLevel2Str
{
    LoggerLevel level;
    const char * str;
};
 
//存储枚举类型到字符串的映射
static LoggerLevel2Str LoggerLevel2StrArr[] = 
{
    {LoggerLevel::DEBUG, "DEBUG"},
    {LoggerLevel::INFO, "INFO"},
    {LoggerLevel::WARN, "WARN"},
    {LoggerLevel::ERROR, "ERROR"},
    {LoggerLevel::FAIL, "FAIL"},
    {LoggerLevel::UNKONW, "UNKONW"}
};

static const char* LoggerLevel2StrFun(LoggerLevel level)
{
    for (int i = 0; i < sizeof(LoggerLevel2StrArr)/sizeof(LoggerLevel2Str); i++)
    {
        if (LoggerLevel2StrArr[i].level == level)
        {
            return LoggerLevel2StrArr[i].str;
        }
        
    }
    
    return "UNKONW";
}


static LoggerLevel Str2LoggerLevelFun(const std::string& str)
{
    for (int i = 0; i < sizeof(LoggerLevel2StrArr)/sizeof(LoggerLevel2Str); i++)
    {
        if (LoggerLevel2StrArr[i].str == str)
        {
            return LoggerLevel2StrArr[i].level;
        }
        
    }
    
    return LoggerLevel::UNKONW;
}


Logger::Logger(const std::string &name) :
    name_(name),
    level_(LoggerLevel::INFO)
{}

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




class  MessageFormatItem : public LoggerFormat::FormatItem
{
public:
    MessageFormatItem(const std::string& str = ""){}

    virtual void format(std::ostream& os, LoggerContent::ptr content) override
    {
        
    }
private:
}

class LevelFormatItem : public LoggerFormat::FormatItem
{
public:
    LevelFormatItem(const std::string& str = ""){}

    virtual void format(std::ostream& os, LoggerContent::ptr content) override
    {
        
    }
private:
}


class ElapseFormatItem : public LoggerFormat::FormatItem
{
public:
    ElapseFormatItem(const std::string& str = ""){}

    virtual void format(std::ostream& os, LoggerContent::ptr content) override
    {
        
    }
private:
}

class NameFormatItem : public LoggerFormat::FormatItem
{
public:
    NameFormatItem(const std::string& str = ""){}

    virtual void format(std::ostream& os, LoggerContent::ptr content) override
    {
        
    }
private:
}

class ThreadIdFormatItem : public LoggerFormat::FormatItem
{
public:
    ThreadIdFormatItem(const std::string& str = ""){}

    virtual void format(std::ostream& os, LoggerContent::ptr content) override
    {
        
    }
private:
}

class NewLineFormatItem : public LoggerFormat::FormatItem
{
public:
    NewLineFormatItem(const std::string& str = ""){}

    virtual void format(std::ostream& os, LoggerContent::ptr content) override
    {
        
    }
private:
}

class NewLineFormatItem : public LoggerFormat::FormatItem
{
public:
    NewLineFormatItem(const std::string& str = ""){}

    virtual void format(std::ostream& os, LoggerContent::ptr content) override
    {
        
    }
private:
}

class DateTimeFormatItem : public LoggerFormat::FormatItem
{
public:
    DateTimeFormatItem(const std::string& str = "%Y-%m-%d %H:%M:%S") : 
        format_(str)
    {
    }

    virtual void format(std::ostream& os, LoggerContent::ptr content) override
    {
        
    }
private:
    std::string format_;
}

class FilenameFormatItem : public LoggerFormat::FormatItem
{
public:
    FilenameFormatItem(const std::string& str = ""){}

    virtual void format(std::ostream& os, LoggerContent::ptr content) override
    {
        
    }
private:
}

class LineFormatItem : public LoggerFormat::FormatItem
{
public:
    LineFormatItem(const std::string& str = ""){}

    virtual void format(std::ostream& os, LoggerContent::ptr content) override
    {
        
    }
private:
}

class TabFormatItem : public LoggerFormat::FormatItem
{
public:
    TabFormatItem(const std::string& str = ""){}

    virtual void format(std::ostream& os, LoggerContent::ptr content) override
    {
        
    }
private:
}

class TabFormatItem : public LoggerFormat::FormatItem
{
public:
    TabFormatItem(const std::string& str = ""){}

    virtual void format(std::ostream& os, LoggerContent::ptr content) override
    {
        
    }
private:
}


class StringFormatItem : public LogFormatter::FormatItem {
public:
    StringFormatItem(const std::string& str) :
        str_(str) 
    {}

    virtual void format(std::ostream& os, LoggerContent::ptr content) override 
    {
        os << m_string;
    }
private:
    std::string str_;
};

class FiberIdFormatItem : public LoggerFormat::FormatItem
{
public:
    FiberIdFormatItem(const std::string& str = ""){}

    virtual void format(std::ostream& os, LoggerContent::ptr content) override
    {
        
    }
private:
}


class ThreadNameFormatItem : public LoggerFormat::FormatItem
{
public:
    ThreadNameFormatItem(const std::string& str = ""){}

    virtual void format(std::ostream& os, LoggerContent::ptr content) override
    {
        
    }
private:

}

LoggerFormat::LoggerFormat(const std::string& pattern) : 
    pattern_(pattern)
{
    init();
}

//LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
void LoggerFormat::init()
{
    //1格式化说明符(str) 2format 3type(0普通的字符串 1其余类型)
    std::vector<std::tuple<std::string, std::string, int>> res;

    size_t len = pattern_.length();

    std::string originStr;  //直接输出的字符串
    for (size_t i = 0; i < len; ++i)
    {
        if (pattern_[i] != '%') //不需要格式化字符串 直接原样输出
        {
            originStr.append(1, pattern_[i]);
            continue;
        }

        if (i+1 < len && pattern_[i+1] == '%')  //处理%%的情况
        {
            originStr.append(1, '%');
            continue;
        }
        
        //此时pattern_[i] = '%' 
        size_t n = i + 1;   //从下一个字符开始
        bool isStart = false;   //是否开始进行fmt的解析标志
        size_t formatStart = 0;//fmt的起始下标
        std::string str;
        std::string fmt;

        while (n < len)
        {
            //没有进行{}的解析 并且当前不是字母也不是{}
            if (!isStart && !std::isalpha(pattern_[n]) && pattern_[n] != '{' && pattern_[n] != '}')
            {
                str = pattern_.substr(i+1, n-i-1);
                break;
            }

            if (!isStart)
            {
                if (pattern_[n] == '{') //开始进行fmt的解析了
                {
                    isStart = true;
                    str = pattern_.substr(i+1, n-i-1);
                    formatStart = n;
                }  
            }
            else
            {
                if (pattern_[n] == '}') //fmt的解析结束了
                {
                    isStart = false;
                    fmt = pattern_.substr(formatStart+1, n-formatStart-1);
                    formatStart = n;
                    ++n;    
                    break;//防止跳到while循环的第一个if里面
                }
            }
            
            ++n;
            if (n == len)
            {
                if (str.empty())    //%后面没有可以解析的内容了 直接把原始字符串输出了
                {
                    str = pattern_.substr(i+1);
                }
                
            }
        }//end while

        if (isStart)    //解析开始了 但是在给定的字符串里面没找到}
        {
            std::cout<<"pattern_ parse error" <<pattern_<<" - "<<pattern_.substr(i)<<std::endl;
            res.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        }
        else
        {
            if (!originStr.empty()) //原始输出的字符串不是空的话
            {
                res.push_back(std::make_tuple(originStr, std::string(), 0));
                originStr.clear();
            }
            
            res.push_back(std::make_tuple(str, fmt, 1));
            i = n-1;    //因为for循环中还要++i
        }
    }// end for

    if (!originStr.empty()) //原始输出的字符串不是空的话
    {
        res.push_back(std::make_tuple(originStr, std::string(), 0));
        originStr.clear();
    }

    //通过格式化说明符去new出来新的item
    static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)>> s_formatItems = {
#define  ITEM(str, C) \
        {#str, [](const std::string& fmt) { return FormatItem::ptr(new C(fmt));}}

        ITEM(m, MessageFormatItem),           //m:消息
        ITEM(p, LevelFormatItem),             //p:日志级别
        ITEM(r, ElapseFormatItem),            //r:累计毫秒数
        ITEM(c, NameFormatItem),              //c:日志名称
        ITEM(t, ThreadIdFormatItem),          //t:线程id
        ITEM(n, NewLineFormatItem),           //n:换行
        ITEM(d, DateTimeFormatItem),          //d:时间
        ITEM(f, FilenameFormatItem),          //f:文件名
        ITEM(l, LineFormatItem),              //l:行号
        ITEM(T, TabFormatItem),               //T:Tab
        ITEM(F, FiberIdFormatItem),           //F:协程id
        ITEM(N, ThreadNameFormatItem),        //N:线程名称

#undef  ITEM
    };
    

    for (auto &v : res)
    {
        if (std::get<2>(v) == 0)    //普通字符串的话
        {
            formats_.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(v))));
        }
        else
        {
            auto it = s_formatItems.find(std::get<0>(v));
            if (s_formatItems.end() == it)
            {
                formats_.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(v) + ">>")));
            }
            else
            {
                formats_.push_back(it->second(std::get<1>(v)));
            }
            
        }
        
    }
    
}

std::string LoggerFormat::format(LoggerContent::ptr content)
{
    std::stringstream ss;
    for (auto it = formats_.begin(); formats_.end() != it; ++it)
    {
        (*it)->format(ss, content);
    }
    
    return ss.str();
}



void LoggerContent::format(const char* fmt, ...)
{
    va_list al;
    va_start(al, fmt);
    format(fmt, al);
    va_end(al);
}


void LoggerContent::format(const char* fmt, va_list al)
{
    char* buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if(len != -1) 
    {
        content_ << std::string(buf, len);
        free(buf);
    }
}


}//end namespace