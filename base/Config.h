#pragma once


#include <memory>
#include <string>
#include <sstream>
#include <functional>

#include <list>
#include <set>
#include <map>
#include <unordered_map>

#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>




#include "Logger.h"


namespace BASE
{


class ConfigVarBase    //配置变量的基类
{
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;

public:
    ConfigVarBase(const std::string& name, const std::string& des = "");
    virtual ~ConfigVarBase() {}

    std::string getName() const {return name_;}
    std::string getDes() const {return descript_;}

    virtual std::string toString() = 0; //将对象序列化成字符串
    virtual bool parseFromString(const std::string& str) = 0;  //从字符串反序列化成对象
    virtual std::string getTypeName() const = 0;
private:
    std::string name_;  //配置名字
    std::string descript_;  //配置描述
};



template <typename From, typename To>
class LexicalCast
{
public:
    To operator()(const From& from)
    {
        return boost::lexical_cast<To>(from);
    }
};


template <typename To>
class LexicalCast<std::string, std::vector<To>>
{
public:
    std::vector<To> operator()(const std::string& str)
    {
        typename std::vector<To> res;
        YAML::Node node = YAML::Load(str);
        std::stringstream ss;
        for (int i = 0; i < node.size(); i++)
        {
            ss.str("");
            ss<<node[i];
            res.push_back(LexicalCast<std::string, To>()(ss.str())); 
        }
        return res;
    }
};

template <typename From>
class LexicalCast<std::vector<From>, std::string>
{
public:
    std::string operator()(const std::vector<From> & vec)
    {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : vec)
        {
            node.push_back(i);
        }
        
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};

template <typename To>
class LexicalCast<std::string, std::list<To>>
{
public:
    std::list<To> operator()(const std::string & str)
    {
        typename std::list<To> res;
        YAML::Node node = YAML::Load(str);
        for (int i = 0; i < node.size(); i++)
        {
            std::stringstream ss("");
            ss<<node[i];
            res.push_back(LexicalCast<std::string, To>()(ss.str()));
        }
        
        return res;
    }
};

template <typename To>
class LexicalCast<std::list<To>, std::string>
{
public:
    std::string operator()(const std::list<To>& l)
    {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : l)
        {
            node.push_back(i);
        }
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};


template <typename To>
class LexicalCast<std::set<To>, std::string>
{
public:
    std::string operator()(const std::set<To>& s)
    {
        std::stringstream ss;
        YAML::Node node (YAML::NodeType::Sequence);
        for (auto &i : s)
        {
            node.push_back(i);
        }
        ss<<node;
        return ss.str();
    }
};

template <typename To>
class LexicalCast<std::string, std::set<To>>
{
public:
    std::set<To> operator()(const std::string& str)
    {
        typename std::set<To> res;
        YAML::Node node = YAML::Load(str);
        for (int i = 0; i < node.size(); i++)
        {
            std::stringstream ss("");
            ss<<node[i];
            res.insert(LexicalCast<std::string, To>()(ss.str()));
        }
        
        return res;
    }
};


template <typename To>
class LexicalCast<std::string, std::map<std::string, To>>
{
public:
    std::map<std::string, To> operator()(const std::string& str)
    {
        typename std::map<std::string, To> res;
        YAML::Node node = YAML::Load(str);

        for (auto it = node.begin(); it != node.end(); ++it)
        {
            std::stringstream ss("");
            ss<<it->second;
            res.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, To>()(ss.str())));
        }
        
        return res;
    }
};

template <typename To>
class LexicalCast<std::map<std::string, To>, std::string>
{
public:
    std::string operator()(const std::map<std::string, To>& m)
    {
        
        YAML::Node node(YAML::NodeType::Map);
        for (auto &i : m)
        {
            node[i.first] = i.second;
        }
        
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};


template<class To>
class LexicalCast<std::string, std::unordered_map<std::string, To> > {
public:
    std::unordered_map<std::string, To> operator()(const std::string& str)
    {
        YAML::Node node = YAML::Load(str);
        typename std::unordered_map<std::string, To> res;
        for(auto it = node.begin(); it != node.end(); ++it) 
        {
            std::stringstream ss("");
            ss << it->second;
            res.insert(std::make_pair(it->first.Scalar(),
                        LexicalCast<std::string, To>()(ss.str())));
        }
        return res;
    }
};


template<class To>
class LexicalCast<std::unordered_map<std::string, To>, std::string> {
public:
    std::string operator()(const std::unordered_map<std::string, To>& m) 
    {
        YAML::Node node(YAML::NodeType::Map);
        for(auto& i : m) 
        {
            node[i.first] = YAML::Load(LexicalCast<To, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


template <typename T, typename ToStr = LexicalCast<T, std::string>, typename FromStr = LexicalCast<std::string, T>>
class ConfigVar;

class Config
{
public:
    typedef std::shared_ptr<Config> ptr;
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigMap;
public:
    
    static bool create(ConfigVarBase::ptr config)  //创建一个配置
    {
        RWmutex::wLock lock(getMutex());
        auto it = getData().find(config->getName());
        if (it != getData().end())  //已经被添加过了
        {
            LOG_ERROR(LOG_ROOT)<<"Config create "<<config->getName()<<"has exist";
            return true;
        }
        
        LOG_ERROR(LOG_ROOT)<<"Config create "<<config->getName()<<" add success";
        getData()[config->getName()] = config;
    }

    template<typename T>
    static typename ConfigVar<T>::ptr lookup(const std::string& name)
    {
        std::string tmpName = name;
        std::transform(tmpName.begin(), tmpName.end(), tmpName.begin(), ::tolower);
        RWmutex::rLock lock(getMutex());
        auto it = getData().find(tmpName);
        if (it == getData().end())  //没有这个配置
        {
            LOG_ERROR(LOG_ROOT)<<"Config lookup "<<name<<"not exist";
            return nullptr;
        }

        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }

    static void loadFromYaml(const YAML::Node& node);
    static ConfigVarBase::ptr getConfigVar(const std::string& name);

private:
    static ConfigMap& getData()
    {
        static ConfigMap data;
        return data;
    }

    static RWmutex& getMutex() //静态成员的初始化顺序问题
    {
        static RWmutex mutex_;
        return mutex_;
    }
    
};


/**
 * T 类型
 * ToStr 把T这个类型序列化成字符串
 * FromStr 把字符串序列化成T
 */
template <typename T, typename ToStr, typename FromStr>
class ConfigVar : public ConfigVarBase
{
public:
    typedef std::shared_ptr<ConfigVar<T>> ptr;
    //配置更改的回调函数
    typedef std::function<void(const T&oldValue, const T&newValue)> ChangeCallback;
public:
    static ptr Create(const std::string& name, const std::string& des, T val) 
    {
        ptr instance(new ConfigVar<T>(name, des, val));
        Config::create(instance);
        return instance;
    }



    virtual std::string toString() override //将对象序列化成字符串
    {
        try
        {
            RWmutex::rLock lock(mutex_);
            return ToStr()(val_);
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(LOG_ROOT)<<"ConfigVar toString Error"<<e.what()<<" convert: "<<typeid(val_).name();
        }
        
        return "";
    }

    virtual bool parseFromString(const std::string& str) override //从字符串反序列化成对象
    {
        try
        {
            setVal(FromStr()(str));
            return true;
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(LOG_ROOT)<<"ConfigVar parseFromString Error"<<e.what()<<" convert: "<<typeid(val_).name();
        }
        
        return false;
    }

    const T getVal() 
    {
        RWmutex::rLock lock(mutex_);
        return val_;
    }
    void setVal(const T& v) 
    {
        RWmutex::wLock lock(mutex_);
        if (v == val_)
        {
            return ;
        }

        for (auto &i : callbacks_)
        {
            i.second(val_, v);
        }
        
        
        val_ = v;
    }

    virtual std::string getTypeName() const override
    {
        return typeid(T).name();
    }

    uint64_t addCallback(ChangeCallback cb)    //注册配置更改的回调函数
    {
        RWmutex::wLock lock(mutex_);
        static uint64_t id = 0;
        ++id;
        callbacks_[id] = cb;
        return id;
    }

    void delCallback(uint64_t id)
    {
        RWmutex::wLock lock(mutex_);
        callbacks_.erase(id);
    }

    ChangeCallback getCallback(uint64_t id)
    {
        RWmutex::rLock lock(mutex_);
        auto it = callbacks_.find(it);
        return it == callbacks_.end() ? nullptr : it.second;
    }

//protected:
    ConfigVar(const std::string& name, const std::string& des, T val):
        ConfigVarBase(name, des),
        val_(val)
    {
    }

private:
    T val_;
    std::map<uint64_t, ChangeCallback> callbacks_;
    RWmutex mutex_;
};








}//end namespace