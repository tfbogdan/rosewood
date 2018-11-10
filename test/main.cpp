#include "Jinx.h"
#include "Jinx.metadata.h"

#include <mc/mc.hpp>
#include <iostream>
#include <functional>

int main() {
    using Jinx = mc::meta_Jinx::meta_jinx::meta_Jinx;
    using JinxTypes = mc::meta_Jinx::meta_jinx::meta_JinxTypes;

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
