#pragma once

#include <string>
#include <vector>
#include <map>

namespace td {

template <typename T>
class SimpleTemplate {      // this shouldn't be exported
public:
    inline std::string stringReturningFoo() {return std::string();}

    T value;
};

template <>
class SimpleTemplate<double> {  // neither should this

private:
    std::string aMember = "";
};

using SimpleTemplateInstance = SimpleTemplate<float>;   // but this should be exported as if SimpleTemplate<float> is a class called SimpleTemplateInstance
using WrappedString = std::string;

using FloatVector = std::vector<float>;

using FloatStringMap = std::map<std::string, float>;


}
