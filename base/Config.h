#pragma once


#include <memory>
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>

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
private:
    std::string name_;  //配置名字
    std::string descript_;  //配置描述
};


template <typename T>
class ConfigVar : public ConfigVarBase
{
public:
    typedef std::shared_ptr<ConfigVar<T>> ptr;
public:
    ConfigVar(const std::string& name, const std::string& des, T val):
        ConfigVarBase(name, des),
        val_(val)
    {}

    virtual std::string toString() override //将对象序列化成字符串
    {
        try
        {
            return boost::lexical_cast<std::string>(val_);
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
            boost::lexical_cast<T>(str);
            return true;
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(LOG_ROOT)<<"ConfigVar parseFromString Error"<<e.what()<<" convert: "<<typeid(val_).name();
        }
        
        return false;
    }

    const T getVal() const {return val_;}
    void setVal(T val) {val_ = val;}
private:
    T val_;
};



class Config
{
public:
    typedef std::shared_ptr<Config> ptr;
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigMap;
public:
    
    template<typename T>
    static bool create(ConfigVarBase::ptr config)  //创建一个配置
    {
        auto it = data_.find(config->getName());
        if (it != data_.end())  //已经被添加过了
        {
            LOG_ERROR(LOG_ROOT)<<"Config create "<<config->getName()<<"has exist";
            return true;
        }
        
        data_[config->getName()] = config;
    }

    template<typename T>
    static typename ConfigVar<T>::ptr lookup(const std::string& name)
    {
        auto it = data_.find(name);
        if (it != data_.end())  //没有这个配置
        {
            LOG_ERROR(LOG_ROOT)<<"Config lookup "<<name<<"not exist";
            return nullptr;
        }

        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }
private:
    static ConfigMap data_;
};




}//end namespace