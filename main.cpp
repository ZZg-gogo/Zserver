#include <iostream>
#include <time.h>
#include <boost/lexical_cast.hpp>

#include <yaml-cpp/yaml.h>
#include <yaml-cpp/node/parse.h>

#include "base/Logger.h"
#include "base/Singleton.h"
#include "base/util.h"
#include "base/Config.h"

BASE::ConfigVar<int>::ptr g_int(new BASE::ConfigVar<int>("system.port", "listenPort", 8080));

BASE::ConfigVar<std::vector<int>>::ptr g_vecInt(new BASE::ConfigVar<std::vector<int>>("vec_int.num", "listenPort", {99,88,77}));

BASE::ConfigVar<std::list<int>>::ptr g_listInt(new BASE::ConfigVar<std::list<int>>("list_int.num", "listenPort", {99,88,77}));

BASE::ConfigVar<std::set<int>>::ptr g_setInt(new BASE::ConfigVar<std::set<int>>("set_int.num", "listenPort", {99,88,77}));

BASE::ConfigVar<std::map<std::string, int>>::ptr g_mapInt\
    (new BASE::ConfigVar<std::map<std::string, int>>("map_int", "listenPort", {{"k", 99},{"z", 77},{"b", 88}}));

BASE::Config::ptr config;

// 递归函数，用于遍历 YAML 节点
void printNode(const YAML::Node& node, int indent = 0) {
    std::string indentation(indent, ' '); // 用于缩进显示

    if (node.IsScalar()) {
        // 如果节点是标量，直接打印
        std::cout << indentation << node.as<std::string>() << std::endl;
    } else if (node.IsSequence()) {
        // 如果节点是序列，遍历每个元素
        for (std::size_t i = 0; i < node.size(); ++i) {
            std::cout << indentation << "- " << std::endl;
            printNode(node[i], indent + 2); // 递归遍历序列中的每个元素
        }
    } else if (node.IsMap()) {
        // 如果节点是映射，遍历每个键值对
        for (auto it = node.begin(); it != node.end(); ++it) {
            std::cout << indentation << it->first.as<std::string>() << ":" << std::endl;
            printNode(it->second, indent + 2); // 递归遍历值
        }
    } else {
        // 处理其他类型
        std::cout << indentation << "Unknown node type" << std::endl;
    }
}



void test()
{
    
#define XX(name)\
{\
    auto v = name->getVal();\
    for (auto & i: v)\
    {\
        LOG_INFO(LOG_ROOT)<<#name<<"---"<<i;\
    };\
    LOG_INFO(LOG_ROOT)<<#name<<"------"<<std::endl<<name->toString();\
}


    XX(g_vecInt)
    XX(g_listInt) 
    XX(g_setInt)

    auto v = g_mapInt->getVal();
    LOG_INFO(LOG_ROOT)<<g_mapInt->toString();\
    for (auto& i : v )
    {
        LOG_INFO(LOG_ROOT)<<i.first<<"---"<<i.second;\
    }

    g_int->addCallback(1, [](const int &old, const int &newValue){
        LOG_INFO(LOG_ROOT)<<"addCallback"<<old<<"---"<<newValue;
    });
    
    YAML::Node node = YAML::LoadFile("./conf/log.yml");
    BASE::Config::loadFromYaml(node);
   
    XX(g_vecInt)
    XX(g_listInt)
    XX(g_setInt)
    v = g_mapInt->getVal();
    for (auto& i : v )
    {
        LOG_INFO(LOG_ROOT)<<i.first<<"---"<<i.second;\
    }
    //printNode(node, 0);
    LOG_INFO(LOG_ROOT)<<g_int->toString();
#undef XX
}

int main(int argc, char ** argv)
{
    LOG_INFO(LOG_ROOT)<<g_int->toString();
    config->create(g_int);
    config->create(g_vecInt); 
    config->create(g_listInt);
    config->create(g_setInt);
    config->create(g_mapInt);
    test();

    BASE::ConfigVar<int> a{"@port", "端口号", 10};
    return 0;
}