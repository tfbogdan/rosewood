#pragma once

#include <string>

namespace td {

template <typename T>
class SimpleTemplate {      // this shouldn't be exported
    T value;
};

template <>
class SimpleTemplate<double> {  // neither should this

private:
    std::string aMember = "";
};

using SimpleTemplateInstance = SimpleTemplate<float>;   // but this should be exported as if SimpleTemplate<float> is a class called SimpleTemplateInstance

}
