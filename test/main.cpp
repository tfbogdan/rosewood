#include "Jinx.h"
#include "Jinx.metadata.h"

#include <mc/mc.hpp>
#include <fmt/printf.h>
#include <iostream>
#include <functional>
#include <memory>

int main() {
    using Jinx = mc::meta<jinx::Jinx>;
    using JinxTypes = mc::meta<jinx::JinxTypes>;

    constexpr Jinx jinx;
    constexpr JinxTypes jinxTypes;

    static_assert (jinx.has_method("aMethod"));
    static_assert (!jinx.has_method("fictionalMethod"));
    static_assert (!jinxTypes.in_range(-27));
    static_assert (jinxTypes.in_range(-32));

    // auto aMethod = jinx.find_method<Jinx::methods>("aMethod");
    // using aMethodType = decltype (aMethod);

    // static_assert (!std::is_same<aMethodType, mc::nil_t>::value);

    jinxTypes.for_each_enumerator([](const auto &en){
        fmt::print("  {}: {:d}\n", en.name, en.value);
    });


    std::unique_ptr<mc::DynamicClass> dynamicJinx(new mc::DynamicClassWrapper<jinx::Jinx>);
    fmt::print("Jinx has a `{}` method: {}\n", "no such method", dynamicJinx->hasMethod("no such method"));
    fmt::print("Jinx has a `{}` method: {}\n", "aMethod", dynamicJinx->hasMethod("aMethod"));
    return 0;
}
