#pragma once

#include <pthread.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <vector>

namespace BASE
{

pid_t getThreadId();    //获取线程id
uint64_t getFiberId(); //获取携程id

//获取堆栈信息放到v中 size是获取多少层 skip要跳过多少层
void Backtrace(std::vector<std::string> & v, int size, int skip);

std::string Backtrace(int size, int skip);    //直接返回一个堆栈信息的字符串


class Time
{
public:
    static time_t now();
    //给定当前时间 获取当前时间所在天的凌晨时间
    static time_t getTodayMidnightTimestamp(time_t now);    
    static std::string strTime(time_t, const std::string& fmt);

    static time_t daySeconds;
};


class FileUtil  //对文件的管理类
{
public:
    
    //创建目录
    static bool mkDir(const std::string& filename);
    //根据filename返回路径名
    static std::string dirName(const std::string& filename);
    //ofstream 文件名 打开方式
    static bool openForWrite(std::ofstream& ofs, const std::string& filename, std::ios_base::openmode mode);
};




}