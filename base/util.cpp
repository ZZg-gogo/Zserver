#include "util.h"

#include <execinfo.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Logger.h"
#include "Fiber.h"

namespace BASE
{

int SetNoBlock(int fd)
{
    int flags = ::fcntl(fd, F_GETFL);
    if (-1 == flags)
    {
        LOG_ERROR(LOG_ROOT)<<"SetNoBlock fcntl Error1";
        return -1;
    }
    

    if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) 
    {
        LOG_ERROR(LOG_ROOT)<<"SetNoBlock fcntl Error2";
        return -1;
    }

    return 0;
}


pid_t getThreadId()   //获取线程id
{
    return syscall(SYS_gettid);
}

uint64_t getFiberId()  //获取携程id
{
    return Fiber::getCurFiberId();
}


//获取堆栈信息放到v中 size是获取多少层 skip要跳过多少层
void Backtrace(std::vector<std::string> & v, int size, int skip = 1)
{
    void ** buf = new void*[size];
    char ** strs = nullptr;

    int count = ::backtrace(buf, size);

    strs = ::backtrace_symbols(buf, count);
    if (nullptr == strs)
    {
        LOG_ERROR(LOG_ROOT)<<"Backtrace Error";
        return;
    }

    for (int i = skip; i < count; i++)
    {
        v.push_back(strs[i]);
    }

    ::free(strs);
    delete[] buf;
}

std::string Backtrace(int size, int skip = 2)    //直接返回一个堆栈信息的字符串
{
    void ** buf = new void*[size];
    char ** strs = nullptr;

    std::string res;
    int count = ::backtrace(buf, size);

    strs = ::backtrace_symbols(buf, count);
    if (nullptr == strs)
    {
        LOG_ERROR(LOG_ROOT)<<"Backtrace Error";
        return {};
    }

    for (int i = skip; i < count; i++)
    {
        res += strs[i];

        res += "\n";
    }

    ::free(strs);
    delete[] buf;
    return std::move(res);
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