#include "BasicDefinitions.h"
#include "BasicDefinitions.metadata.h"
#include "TemplateDeclarations.h"
#include "TemplateDeclarations.metadata.h"

#include <mc/dynamic_mc.hpp>
#include <mc/type.hpp>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

struct noArgsNoReturnPredicate{
    static constexpr std::string_view pred = "noArgsNoReturnMethod";
};

TEST(mc, static_enumeration) {
    using Enum = mc::meta<basic::Enum>;
    constexpr Enum enumInst;

    const std::vector<std::pair<std::string, int>> expectedEnums {
        std::pair("negativeEnumerator", -32),
        std::pair("nextNegativeEnumerator", -31),
        std::pair("zeroEnumerator", 0),
        std::pair("oneEnumerator", 1),
        std::pair("hundredEnumerator", 100)
    };

    std::vector<std::pair<std::string, int>> jinxEnumerators;
    enumInst.for_each_enumerator([&jinxEnumerators](const auto &en){
        jinxEnumerators.emplace_back(en.get_name(), en.value);
    });

    EXPECT_EQ (jinxEnumerators, expectedEnums);

    static_assert (!enumInst.in_range(-27));
    static_assert (enumInst.in_range(-32));

    EXPECT_TRUE(enumInst.in_range(-31));
    EXPECT_FALSE(enumInst.in_range(222));
}


TEST(mc, static_class) {
    using PlainClass = mc::meta<basic::PlainClass>;
    constexpr PlainClass plainClass;

    static_assert (!plainClass.has_overload_set("aMethod"));
    static_assert (!plainClass.has_overload_set("fictionalMethod"));
    static_assert (plainClass.has_overload_set("noArgsNoReturnMethod"));

    const std::vector<std::string> expectedJinxMethods {
        "noArgsNoReturnMethod",
        "doubleInteger",
        "overloadedMethod"
    };

    std::vector<std::string> jinxMethods;

    plainClass.visit_overload_sets([&jinxMethods](auto overloadSet) constexpr {
        jinxMethods.emplace_back(overloadSet.get_name());
    });

    auto noArgsNoReturnMethod = plainClass.get_overload_set(noArgsNoReturnPredicate());
    static_assert (noArgsNoReturnMethod.num_overloads() == 1);
    auto onlyOverload = noArgsNoReturnMethod.get_overload<0>();
    static_assert (onlyOverload.num_params() == 0);
}

TEST(mc, runtime_module) {
    using BasicDefinitions = mc::DNamespaceWrapper<mc::meta_BasicDefinitions>;
    BasicDefinitions basicDefs;

    EXPECT_EQ(basicDefs.getNamespaces()[0]->getName(), "basic");
    EXPECT_EQ(basicDefs.getClasses().size(), 0);
    EXPECT_EQ(basicDefs.getEnums().size(), 0);

    const mc::DNamespace *dBasic = basicDefs.getNamespaces()[0];

    EXPECT_EQ(dBasic->getNamespaces().size(), 0);
    EXPECT_EQ(dBasic->getEnums().size(), 2);
    EXPECT_EQ(dBasic->getClasses().size(), 2);
}

TEST(mc, runtime_namespace) {
    using BasicDefinitions = mc::DNamespaceWrapper<mc::meta_BasicDefinitions>;
    BasicDefinitions basicDefs;

    const mc::DNamespace *dBasic = basicDefs.getNamespaces()[0];

    EXPECT_THROW(dBasic->findChildNamespace("unthinkable"), std::out_of_range);
    EXPECT_NO_THROW(basicDefs.findChildNamespace("basic"));
    EXPECT_EQ(basicDefs.findChildNamespace("basic"), dBasic);
}

TEST(mc, runtime_class) {
    std::unique_ptr<mc::DClass> dynamicJinx(new mc::DClassWrapper<mc::meta<basic::PlainClass>>);
    EXPECT_FALSE(dynamicJinx->hasMethod("no such method"));
    EXPECT_TRUE(dynamicJinx->hasMethod("overloadedMethod"));
}

TEST(mc, runtime_searches) {
    using BasicDefinitions = mc::DNamespaceWrapper<mc::meta_BasicDefinitions>;
    BasicDefinitions basicDefs;

    EXPECT_NO_THROW(basicDefs.findChildNamespace("basic")->findChildClass("PlainClass")->findOverloadSet("doubleInteger"));
    auto aMethodSet = basicDefs.findChildNamespace("basic")->findChildClass("PlainClass")->findOverloadSet("doubleInteger");
    auto aMethod = aMethodSet->getMethods()[0];

    int aMethodRes;
    int aMethodArg = 12;
    void *aMethodArgs[] = {&aMethodArg};
    basic::PlainClass plainClass;

    aMethod->call(&plainClass, &aMethodRes, aMethodArgs);
    EXPECT_EQ(aMethodRes, plainClass.doubleInteger(aMethodArg));
    EXPECT_THROW(aMethod->call(&static_cast<const basic::PlainClass&>(plainClass), &aMethodRes, aMethodArgs), mc::const_corectness_error);
}

TEST(mc, string_wrap) {
    using TemplateDeclarations = mc::DNamespaceWrapper<mc::meta_TemplateDeclarations>;
    TemplateDeclarations module;
    auto tdNamespace = module.findChildNamespace("td");
    auto strWrapper = tdNamespace->findChildClass("WrappedString");
    const std::string testString = "Hello World!";
    char *res;
    auto cStr = strWrapper->findOverloadSet("c_str")->getMethods()[0];
    cStr->call(&testString, &res, nullptr);
    EXPECT_EQ(testString, res);

    static_assert(std::is_same<mc::const_ref<int>::type, const int&>::value);
    static_assert (std::is_same<mc::const_ref<int>::atomic_type, int>::value);
}
