#include "Jinx.h"
#include "Jinx.metadata.h"

#include <mc/dynamic_mc.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

struct predicate {
    static constexpr std::string_view pred = "aMethod";
};

TEST(mc, static_enumeration) {
    using JinxTypes = mc::meta<jinx::JinxTypes>;
    constexpr JinxTypes jinxTypes;

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

    static_assert (!jinxTypes.in_range(-27));
    static_assert (jinxTypes.in_range(-32));

    EXPECT_TRUE(jinxTypes.in_range(-31));
    EXPECT_FALSE(jinxTypes.in_range(222));
}


TEST(mc, static_class) {
    using Jinx = mc::meta<jinx::Jinx>;
    constexpr Jinx jinx;

    static_assert (jinx.has_overload_set("aMethod"));
    static_assert (!jinx.has_overload_set("fictionalMethod"));

    const std::vector<std::string> expectedJinxMethods {
        "aMethod",
        "overloadedMethod"
    };

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


    auto aMethod = jinx.get_overload_set(predicate());
    static_assert (aMethod.num_overloads() == 1);
    auto onlyOverload = aMethod.get_overload<0>();
    static_assert (onlyOverload.num_params() == 1);
    auto onlyParam = onlyOverload.get_param<0>();
    static_assert (std::is_same<decltype(onlyParam)::type, int>::value);
    static_assert (onlyParam.get_name() == "namedParam");
}

TEST(mc, runtime_module) {
    using JinxModule = mc::DNamespaceWrapper<mc::meta_Jinx>;
    JinxModule module;

    EXPECT_EQ(module.getNamespaces()[0]->getName(), "jinx");
    EXPECT_EQ(module.getClasses().size(), 0);
    EXPECT_EQ(module.getEnums().size(), 0);

    const mc::DNamespace *dJinx = module.getNamespaces()[0];

    EXPECT_EQ(dJinx->getNamespaces().size(), 0);
    EXPECT_EQ(dJinx->getEnums().size(), 2);
    EXPECT_EQ(dJinx->getClasses().size(), 1);
}

TEST(mc, runtime_namespace) {
    using JinxModule = mc::DNamespaceWrapper<mc::meta_Jinx>;
    JinxModule module;

    const mc::DNamespace *dJinx = module.getNamespaces()[0];

    EXPECT_EQ(dJinx->getNamespaces().size(), 0);
    EXPECT_EQ(dJinx->getEnums().size(), 2);
    EXPECT_EQ(dJinx->getClasses().size(), 1);

    EXPECT_THROW(dJinx->findChildNamespace("unthinkable"), std::out_of_range);
    EXPECT_NO_THROW(module.findChildNamespace("jinx"));
    EXPECT_EQ(module.findChildNamespace("jinx"), dJinx);
}

TEST(mc, runtime_class) {
    std::unique_ptr<mc::DClass> dynamicJinx(new mc::DClassWrapper<mc::meta<jinx::Jinx>>);
    EXPECT_FALSE(dynamicJinx->hasMethod("no such method"));
    EXPECT_TRUE(dynamicJinx->hasMethod("aMethod"));
}

TEST(mc, runtime_searches) {
    using JinxModule = mc::DNamespaceWrapper<mc::meta_Jinx>;
    JinxModule module;

    EXPECT_NO_THROW(module.findChildNamespace("jinx")->findChildClass("Jinx")->findOverloadSet("aMethod"));
    auto aMethodSet = module.findChildNamespace("jinx")->findChildClass("Jinx")->findOverloadSet("aMethod");
    auto aMethod = aMethodSet->getMethods()[0];
    (void) aMethod;
}
