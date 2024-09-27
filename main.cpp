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
    auto v = g_vecInt->getVal();
    for (auto & i: v)
    {
        LOG_INFO(LOG_ROOT)<<i;
    }
    YAML::Node node = YAML::LoadFile("./conf/log.yml");
    BASE::Config::loadFromYaml(node);
   
    v = g_vecInt->getVal();
    for (auto & i: v)
    {
        LOG_INFO(LOG_ROOT)<<i;
    }
    //printNode(node, 0);
    LOG_INFO(LOG_ROOT)<<g_int->toString();
}

int main(int argc, char ** argv)
{
    LOG_INFO(LOG_ROOT)<<g_int->toString();
    config->create(g_int);
    config->create(g_vecInt);
    test();

    BASE::ConfigVar<int> a{"@port", "端口号", 10};
    return 0;
}