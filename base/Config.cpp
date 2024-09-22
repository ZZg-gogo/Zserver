#include "Config.h"

namespace BASE
{


ConfigVarBase::ConfigVarBase(const std::string& name, const std::string& des) :
    name_(name),
    descript_(des)
{
    std::transform(name_.begin(), name_.end(), name_.begin(), ::tolower);
}


Config::ConfigMap Config::data_;

}

