#include "BasicDefinitions.h"
#include "BasicDefinitions.metadata.h"
#include "TemplateDeclarations.h"
#include "TemplateDeclarations.metadata.h"

#include <rosewood/runtime.hpp>
#include <rosewood/index.hpp>

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
    using Enum = rosewood::meta<basic::Enum>;
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
    using PlainClass = rosewood::meta<basic::PlainClass>;
    constexpr PlainClass plainClass;

    static_assert (!plainClass.has_method("aMethod"));
    static_assert (!plainClass.has_method("fictionalMethod"));
    static_assert (plainClass.has_method("noArgsNoReturnMethod"));

    const std::vector<std::string> expectedJinxMethods {
        "noArgsNoReturnMethod",
        "doubleInteger",
        "overloadedMethod"
    };

    std::vector<std::string> jinxMethods;

    plainClass.visit_methods([&jinxMethods](auto method) constexpr {
        jinxMethods.emplace_back(method.name);
    });
}

TEST(mc, runtime_module) {
    constexpr rosewood::meta_BasicDefinitions sbd;
    rosewood::DNamespaceWrapper basicDefs(sbd, nullptr);

//    EXPECT_EQ(basicDefs.getNamespaces()[0]->getName(), "basic");
//    EXPECT_EQ(basicDefs.getClasses().size(), 0);
//    EXPECT_EQ(basicDefs.getEnums().size(), 0);
    EXPECT_FALSE(basicDefs.getDeclaration("basic")->asNamespace() == nullptr);

    const rosewood::DNamespace *dBasic = basicDefs.getDeclaration("basic")->asNamespace();

//    EXPECT_EQ(dBasic->getNamespaces().size(), 0);
//    EXPECT_EQ(dBasic->getEnums().size(), 2);
//    EXPECT_EQ(dBasic->getClasses().size(), 2);
}

TEST(mc, runtime_namespace) {
    constexpr rosewood::meta_BasicDefinitions rbd;
    rosewood::DNamespaceWrapper basicDefs(rbd, nullptr);

    const rosewood::DNamespace *dBasic = basicDefs.getDeclaration("basic")->asNamespace();

    EXPECT_EQ(dBasic->getDeclaration("unthinkable"), nullptr);
}

TEST(mc, runtime_searches) {
    constexpr rosewood::meta_BasicDefinitions rbd;
    rosewood::DNamespaceWrapper basicDefs(rbd, nullptr);

    EXPECT_TRUE(basicDefs.getDeclaration("basic")->asNamespace() != nullptr);
    auto basicNmpsc = basicDefs.getDeclaration("basic")->asNamespace();

    EXPECT_TRUE(basicNmpsc->getDeclaration("PlainClass")->asClass() != nullptr);
    auto plainClss = basicNmpsc->getDeclaration("PlainClass")->asClass();

    EXPECT_TRUE(plainClss->getDeclaration("doubleInteger")->asMethod() != nullptr);
    auto dblIntgrMtd = plainClss->getDeclaration("doubleInteger")->asMethod();

    EXPECT_TRUE(plainClss->getDeclaration("intField")->asField() != nullptr);
    auto intField = plainClss->getDeclaration("intField")->asField();

    int aMethodRes;
    int aMethodArg = 12;
    void *aMethodArgs[] = {&aMethodArg};
    basic::PlainClass plainClass;

    int testValue = 1337;
    intField->assign_copy(&plainClass, &testValue);
    dblIntgrMtd->call(&plainClass, &aMethodRes, aMethodArgs);
    EXPECT_EQ(plainClass.intField, testValue);

    EXPECT_EQ(aMethodRes, plainClass.doubleInteger(aMethodArg));
    EXPECT_NO_THROW(dblIntgrMtd->call(&static_cast<const basic::PlainClass&>(plainClass), &aMethodRes, aMethodArgs));
}

TEST(mc, string_wrap) {
    constexpr rosewood::meta_TemplateDeclarations tds;
    rosewood::DNamespaceWrapper module(tds, nullptr);

    auto tdNamespace = module.getDeclaration("td")->asNamespace();

    auto strWrapper = tdNamespace->getDeclaration("WrappedString")->asClass();

    const std::string testString = "Hello World!";
    char *res;
    auto cStr = strWrapper->getDeclaration("c_str")->asMethod();

    cStr->call(&testString, &res, nullptr);
    EXPECT_EQ(testString, res);
}

TEST(mc, index) {
    using Index = rosewood::StaticIndex<rosewood::meta_BasicDefinitions, rosewood::meta_TemplateDeclarations>;
    Index index;

    static constexpr auto method =
            rosewood::MethodDeclaration(
                &basic::PlainClass::constNoExceptFunction,
                "constNoExceptFunction",
                std::tuple<>());
    static constexpr auto otherMethod =
           rosewood::MethodDeclaration(
              &basic::PlainClass::doubleInteger,
              "doubleInteger",
              std::tuple(rosewood::FunctionParameter<int>("namedParam", false, 0)));
    EXPECT_TRUE(method.isConst());
    EXPECT_TRUE(otherMethod.isConst());
    const basic::PlainClass plainClass;
    method.invoke(&plainClass, nullptr, nullptr);

    int returnSlot;
    int argValue = 6;
    void* argsArray[] = {&argValue};

    otherMethod.invoke(&plainClass, &returnSlot, argsArray);
    EXPECT_EQ(returnSlot, plainClass.doubleInteger(argValue));
}
