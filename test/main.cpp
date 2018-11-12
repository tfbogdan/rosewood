#include "Jinx.h"
#include "Jinx.metadata.h"

#include <mc/mc.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>


TEST(MC, compiletime) {
    using Jinx = mc::meta<jinx::Jinx>;
    using JinxTypes = mc::meta<jinx::JinxTypes>;

    constexpr Jinx jinx;
    constexpr JinxTypes jinxTypes;

    static_assert (jinx.has_method("aMethod"));
    static_assert (!jinx.has_method("fictionalMethod"));
    static_assert (!jinxTypes.in_range(-27));
    static_assert (jinxTypes.in_range(-32));

    std::vector<std::pair<std::string, int>> jinxEnumerators;

    jinxTypes.for_each_enumerator([&jinxEnumerators](const auto &en){
        jinxEnumerators.emplace_back(en.name, en.value);
    });

    const std::vector<std::pair<std::string, int>> expectedEnums {
        std::pair("underJinx", -32),
        std::pair("overUnderJinx", -31),
        std::pair("zeroJinx", 0),
        std::pair("moreJinx", 1),
        std::pair("superJinx", 100)
    };

    EXPECT_EQ (jinxEnumerators, expectedEnums);

    std::unique_ptr<mc::DynamicClass> dynamicJinx(new mc::DynamicClassWrapper<jinx::Jinx>);
    EXPECT_FALSE(dynamicJinx->hasMethod("no such method"));
    EXPECT_TRUE(dynamicJinx->hasMethod("aMethod"));
}
