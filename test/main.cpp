#include "Jinx.h"
#include "Jinx.metadata.h"

#include <mc/mc.hpp>
#include <iostream>
#include <functional>

int main() {
    using Jinx = mc::meta<jinx::Jinx>::type;
    using JinxTypes = mc::meta<jinx::JinxTypes>::type;

    constexpr Jinx jinx;
    constexpr JinxTypes jinxTypes;

    static_assert (jinx.has_method("aMethod"));
    static_assert (!jinx.has_method("fictionalMethod"));
    static_assert (!jinxTypes.in_range(-27));
    static_assert (jinxTypes.in_range(-32));

    jinxTypes.for_each_enumerator([](const auto &en){
        std::cout << "  " << en.name << ": " << en.value << "\n";
    });

    return 0;
}
