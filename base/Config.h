#pragma once


#include <memory>
#include <string>
#include <sstream>
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



/**
 * T 类型
 * ToStr 把T这个类型序列化成字符串
 * FromStr 把字符串序列化成T
 */
template <typename T, typename ToStr = LexicalCast<T, std::string>, typename FromStr = LexicalCast<std::string, T>>
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

    static void loadFromYaml(const YAML::Node& node);
    static ConfigVarBase::ptr getConfigVar(const std::string& name);
private:
    static ConfigMap data_;
};




}//end namespace