#pragma once

#include <pthread.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>

namespace BASE
{

pid_t getThreadId();    //获取线程id
u_int32_t getFiberId(); //获取携程id


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