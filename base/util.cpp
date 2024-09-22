#include "util.h"

#include <string.h>
#include <sys/stat.h>

namespace BASE
{

pid_t getThreadId()   //获取线程id
{
    return syscall(SYS_gettid);
}

u_int32_t getFiberId()  //获取携程id
{
    return 0;
}


static int l_lstat(const char *filename, struct stat *st = nullptr)
{
    struct stat lst;
    int ret = ::lstat(filename, &lst);
    if (nullptr != st)
    {
        *st = lst;
    }

    if (ret < 0)
    {
        return -1;
    }
    
    return 0;
}

static int l_mkdir(const char * filename)
{
    if (0 == access(filename, F_OK))
    {
        return 0;
    }
    
    return ::mkdir(filename, 0777);
}


bool FileUtil::openForWrite(std::ofstream& ofs, const std::string& filename, std::ios_base::openmode mode)  
{
    ofs.open(filename, mode);
    if (!ofs.is_open())
    {
        std::string dir = dirName(filename);
        if (!mkDir(dir))
        {
            return false;
        }
        ofs.open(filename, mode);
    }
    

    return ofs.is_open();
}

//创建文件前面的路径
bool FileUtil::mkDir(const std::string& dirName)
{
    if (0 == l_lstat(dirName.c_str()))  //如果前面的路径已经存在的话
    {
        return true;
    }

    char * path = strdup(dirName.c_str());  //将原路径拷贝一份
    char *p = strchr(path+1, '/');    //找到第一个/的位置
    bool flag = true;

    for (; p; *p='/', p = strchr(p+1, '/'))
    {
        *p = '\0';
        if (0 != l_mkdir(path)) //如果创建本级目录失败的话
        {
            flag = false;
            break;
        }
        
    }   //出来之后 /a/b/c/1.txt 前面的 /a/b/c就已经创建好了
    

    if (p != nullptr)   //输入的路径是错误的
    {
        flag = false;
    }
    
    free(path);

    if (flag)
    {
        return 0 == l_mkdir(dirName.c_str());   //创建最后的1.txt
    }
    return flag;  
}

//根据filename返回路径名
std::string FileUtil::dirName(const std::string& filename)
{
    size_t pos = filename.find_last_of('/');
    if (std::string::npos == pos)  //当前路径下
    {
        return ".";
    }
    else if (0 == pos)  //根目录下
    {
        return "/";
    }
    else
    {
        return filename.substr(0, pos);
    } 
}


time_t Time::daySeconds = 60*60*24;

time_t Time::now()
{
    return ::time(nullptr);
}

time_t Time::getTodayMidnightTimestamp(time_t now)
{
    tm t;
    ::localtime_r(&now, &t);

    t.tm_hour = 0;
    t.tm_min = 0;
    t.tm_sec = 0;

    return ::mktime(&t);
}


std::string Time::strTime(time_t timestamp, const std::string& fmt)
{
    char timeBuf[60] = {};
    tm t;
    ::localtime_r(&timestamp, &t);
    strftime(timeBuf, sizeof(timeBuf), fmt.c_str(), &t);

    return timeBuf;
}

}