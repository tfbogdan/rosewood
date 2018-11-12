#include "Jinx.h"
#include "Jinx.metadata.h"

#include <mc/dynamic_mc.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>


TEST(MC, compiletime) {
    using sv = std::string_view;

    using Jinx = mc::meta<jinx::Jinx>;
    using JinxTypes = mc::meta<jinx::JinxTypes>;

    constexpr Jinx jinx;
    constexpr JinxTypes jinxTypes;

    constexpr sv aMethod = "aMethod";
    static_assert (jinx.has_overload_set(aMethod));
    static_assert (!jinx.has_overload_set("fictionalMethod"));
    static_assert (!jinxTypes.in_range(-27));
    static_assert (jinxTypes.in_range(-32));


    const std::vector<std::pair<std::string, int>> expectedEnums {
        std::pair("underJinx", -32),
        std::pair("overUnderJinx", -31),
        std::pair("zeroJinx", 0),
        std::pair("moreJinx", 1),
        std::pair("superJinx", 100)
    };

    std::vector<std::pair<std::string, int>> jinxEnumerators;
    jinxTypes.for_each_enumerator([&jinxEnumerators](const auto &en){
        jinxEnumerators.emplace_back(en.get_name(), en.value);
    });


    EXPECT_EQ (jinxEnumerators, expectedEnums);

    const std::vector<std::string> expectedJinxMethods {
        "aMethod",
        "overloadedMethod"
    };
    EXPECT_TRUE(jinxTypes.in_range(-31));
    EXPECT_FALSE(jinxTypes.in_range(222));

    std::vector<std::string> jinxMethods;


    jinx.visit_overload_sets([&jinxMethods](auto overloadSet) constexpr {
        EXPECT_NE(overloadSet.get_name(), "");
        jinxMethods.emplace_back(overloadSet.get_name());
        if (overloadSet.get_name() == "aMethod") {
            EXPECT_EQ(overloadSet.num_overloads(), 1);
            overloadSet.for_each_overload([](auto overload){
                EXPECT_EQ(overload.num_params(), 1);
                overload.for_each_parameter([](auto param){
                    EXPECT_EQ(param.get_name(), "namedParam");
                });
            });
        }
    });

    std::unique_ptr<mc::DynamicClass> dynamicJinx(new mc::DynamicClassWrapper<jinx::Jinx>);
    EXPECT_FALSE(dynamicJinx->hasMethod("no such method"));
    EXPECT_TRUE(dynamicJinx->hasMethod("aMethod"));
}
