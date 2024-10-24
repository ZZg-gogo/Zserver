#include "Config.h"
#include <list>
#include <sstream>

namespace BASE
{

static bool isInvalidName(const std::string& name)
{
    return name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678")
            != std::string::npos;
}

ConfigVarBase::ConfigVarBase(const std::string& name, const std::string& des) :
    name_(name),
    descript_(des)
{
    std::transform(name_.begin(), name_.end(), name_.begin(), ::tolower);
    if(isInvalidName(name_))
    {
        LOG_ERROR(LOG_ROOT)<<"ConfigVarBase is invalid "<<name_;
    }
    
}


static void listAllYamlMembers(const std::string& prefix, 
                               const YAML::Node& node,
                               std::list<std::pair<std::string, const YAML::Node>>& output)
{
    if (isInvalidName(prefix))
    {
        LOG_ERROR(LOG_ROOT)<<"listAllYamlMembers isInvalidName "<<prefix<<" - "<<node;
        return;
    }
    
    output.push_back(std::make_pair(prefix, node));
    if (node.IsMap())
    {
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            listAllYamlMembers(prefix.empty()? it->first.Scalar(): prefix + "." + it->first.Scalar(),
                              it->second, output);
        }
        
    }
    
}




void Config::loadFromYaml(const YAML::Node& node)
{

    std::list<std::pair<std::string, const YAML::Node>> allNodes;
    listAllYamlMembers("", node, allNodes);

    for (auto it = allNodes.begin(); it != allNodes.end(); ++it)
    {
        if (it->first.empty())
        {
            continue;
        }
        
        
        ConfigVarBase::ptr var = getConfigVar(it->first);
        if (var)
        {
            if (it->second.IsScalar())  //如果是字符串的话
            {
                var->parseFromString(it->second.Scalar());
            }
            else
            {
                std::stringstream ss;
                ss<<it->second;
                var->parseFromString(ss.str());
            }
            
        }

        
    }
    
}

ConfigVarBase::ptr Config::getConfigVar(const std::string& name)
{
    std::string tmpName = name;
    std::transform(tmpName.begin(), tmpName.end(), tmpName.begin(), ::tolower);
    RWmutex::rLock lock(getMutex());
    auto it = getData().find(tmpName);
    return getData().end() == it ? nullptr : it->second;
}


}

