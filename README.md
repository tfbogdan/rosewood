# mc [![Build Status](https://travis-ci.org/tfbogdan/mc.svg?branch=master)](https://travis-ci.org/tfbogdan/mc) [![codecov](https://codecov.io/gh/tfbogdan/mc/branch/master/graph/badge.svg)](https://codecov.io/gh/tfbogdan/mc)
A no nonsense approach to C++ reflection. Aims to provide the means of accessing type information at compile and run time. Achieves that by means of a Clang tool that generates a compile time model of the code it parses. Tries to stay out of its users way by not inventing new syntax or high level concepts such as signals and slots but enables the (more accessible) possibility of higher level constructs in user code.

## Note
This project is in a very early state so don't attempt to use it if you expect consistent results or even any results at all. If you enjoy experimenting with unfinished prototypes than by all means go crazy with it! If you feel like this could be valuable to you then you are welcome to contribute. Something as simple as installing and using the package and reporting an issue you've found can kick it forward or even just providing your opinion after seeing this readme but contributing code is even better. Right now, too little is tested and a huge number of use cases are just not taken into account.

## Introduction
Generating reflection data for all public entities in a file is as simple as:

```cmake
find_package(mc REQUIRED)

metacompile_header(<your target> <your header file>)
```

That's it! Check out the rules that are used when the tool determines which declarations are exported in [[docs/export_rules.md]].Doing that alone and then including the generated file in your code is all you need to have full compile time access over the definitions in the header. If you do not want runtime access (or you don't find the way it is provided by this framework suitable and you'd rather build your own representation of it using the compile time model) then you don't even need to link any libraries (_This is only partly true today but that's where the project is headed_). Then you can do stuff such as:

```c++
// in Jinx.h

namespace jinx {
    enum JinxTypes : int8_t {
        underJinx = -32,
        overUnderJinx,

        zeroJinx = 0,
        moreJinx,
        superJinx = 100
    };
}

// in main.cpp

#include "Jinx.h"
#include "Jinx.metadata.h"
#include <iostream>

int main() {
  using JinxTypes = mc::meta<jinx::JinxTypes>;
  constexpr JinxTypes jinxTypes;
  jinxTypes.for_each_enumerator([](const auto en){
      std::cout << en.get_name() << ": " << int(en.get_value()) << "\n";
  });
  return 0;
}

// would generate the output:
underJinx: -32
overUnderJinx: -31
zeroJinx: 0
moreJinx: 1
superJinx: 100

```

## Getting started

As of now there is no binary distribution of this suite so you'll have to build it from source. It relies on standard C++ 17 so it should compile on any platform with a working C++17 compiler and if it doesn't it will be hastily resolved once reported. You'll also need:

- A recent (7+) version of the clang libraries, and I mean the C++ libraries not the C wrapper. You'll be able to find precompiled binaries for a wide range of platforms at the [LLVM Release page](http://releases.llvm.org/download.html#7.0.0). If you're not on one of the targeted platforms then don't despair as it's quite easy to compile LLVM and Clang from source. Follow the [LLVM Getting Started guide](https://llvm.org/docs/GettingStarted.html) and you'll be up and running in a heartbeat. You'll find the trickyest part of building them is having the patience for the build to be over and (be warned!) if compiling debug binaries finding the disk space necessary.
- Conan, as the rest of dependencies are fortunately available in conan center. Installing conan is as simple as `pip install conan`.
- Cmake >= 3.9.

Once all of the above criteria are met and you're sure cmake knows how to find Clang when `find_package(Clang CONFIG)` is called by the CMakeLists during configuration then you're good to go. The rest is just a matter of compiling a cmake project.

## Compile time model

Is essentially a huge nested struct per file, that approximately resembles:

```c++

namespace mc {
  struct meta_Jinx : public Namespace<Jinx> { // entry point for a file called Jinx.h
    static constexpr std::string_view name = "Jinx";  // every generated descriptor gets a name.
                                                      // For the rest of this example it will be skipped but in actual code it's always there.

    struct meta_JinxTypes : public Enum<JinxTypes> { // for an enum called say JinxTypes
      struct meta_FirstEnumerator : public Enumerator<FirstEnumerator> { // each enumerator gets one of these
        static constexpr int value = 12;  // the type depends on the enum class type and value is the value of the enumerator in the enum itself
      };

      using enumerators = std::tuple<meta_JinxTypes>;  // generally, after declaring entities statically they are collected in a tuple type
                                                  // so they are fully usable in generic code. the tuple type is appropriately called enumerators in this case
    };

    struct meta_JinxClass : public Class<JinxClass> { // a class called JinxClass you'd get a descriptor resembling this one
      struct meta_someMethod : public OverloadSet<meta_someMethod> {  // and for a method called `someMethod` (even one without overloads) you'd get what is called an overload set
        struct meta_overload_1 : public Method<meta_overload1> {  // ultimately listing every overloads of a method like this
          inline static void fastcall(void *obj, void *returnAddress, void **args) {
            // and every method gets some dispatchers. this one will construct a method call from the
            // void pointers provided without making any checks. It assumes the
            // user already made all the checks necessary which makes it dangerous but also very fast.
            /*... actual code not so interesting... */
          }
          // following the pattern you're also provided with the argument list
          using parameters = std::tuple</*parameter descriptors*/>;
        };
        using overloads = std::tuple<meta_overload_1>; // and all overloads are listed
      };

      using overload_sets = std::tuple<meta_someMethod>; // naturally all overload sets are listed too
      using fields = std::tuple</*fields of the class*/>; // or fields

      struct constructor : public OverloadSet<constructor> {}; // constructors aren't left out either
      struct destructor : public Method<destructor> {}; // or the destructor

      using enums = std::tuple</*or enums if the class defines any*/>;
      using classes = std::tuple</*same for classes*/>;
    };

    using enums = ...;// the pattern is applied for everything and the entire model is entirely traversable during compile time;
  };

// In addition, the mc::meta template also gets a specialization that becomes one of the descriptors from above depending on the type it is specialized over
// so it becomes easy to reference in user code
// namespaces don't get these so you'll have to traverse from the root of the model to find them
template<>
struct meta<jinx::JinxTypes> : public mc::meta_Jinx::metaJinxTypes {};
}

```

# Runtime model

Based on the compile time model described above, a runtime model is also provided. It's defined in terms of pure abstract classes such as:

```c++

    class DNamespace {
    public:
        virtual ~DNamespace() = 0;

        virtual const std::vector<const DNamespace*> getNamespaces() const noexcept = 0;
        virtual const std::vector<const DEnum*> getEnums() const noexcept = 0;
        virtual const std::vector<const DClass*> getClasses() const noexcept = 0;
        virtual const DNamespace *findChildNamespace(std::string_view name) const noexcept(false) = 0;
    };

    class DMethod {
    public:
        virtual ~DMethod() = 0;

        virtual void call(const void *object, void *retValAddr, void **args) const = 0;
        virtual void call(void *object, void *retValAddr, void **args) const = 0;
    private:

    };

```

Instantiating these is possible by using a series of templated wrappers that you can use like this (excerpt from the test suite of the project)

```c++

    using JinxModule = mc::DNamespaceWrapper<mc::meta_Jinx>;  // get an instance for the root of a model
    JinxModule module;

    EXPECT_EQ(module.getNamespaces()[0]->getName(), "jinx");  // and get the first namespace of that model. in our test code, that is called jinx
    EXPECT_EQ(module.getClasses().size(), 0);                 // we know the root of the module has no classes
    EXPECT_EQ(module.getEnums().size(), 0);                   // or enums

    const mc::DNamespace *dJinx = module.getNamespaces()[0];

    EXPECT_EQ(dJinx->getNamespaces().size(), 0);              // no namespaces nested within jinx
    EXPECT_EQ(dJinx->getEnums().size(), 2);                   // but we do have two enums
    EXPECT_EQ(dJinx->getClasses().size(), 1);                 // and a class

    EXPECT_THROW(dJinx->findChildNamespace("unthinkable"), std::out_of_range);  // asking for a namespace that doesn't exist throws an exception
    EXPECT_NO_THROW(module.findChildNamespace("jinx"));                         // if it exists then it's politely returned back to the called
    EXPECT_EQ(module.findChildNamespace("jinx"), dJinx);                        // and we know for a fact that the jinx namespace has position 0 for this test

    std::unique_ptr<mc::DClass> dynamicJinx(new mc::DClassWrapper<mc::meta<jinx::Jinx>>); // and now we're creating an instance for the Jinx class. Could also search for it in the module. It's already nested within.
    EXPECT_FALSE(dynamicJinx->hasMethod("no such method"));                     // no such method, friend!
    EXPECT_TRUE(dynamicJinx->hasMethod("aMethod"));                             // but we know this one exists

    auto aMethodSet = dynamicJinx->findOverloadSet("aMethod");
    auto aMethod = aMethodSet->getMethods()[0];

    int aMethodRes;
    int aMethodArg = 12;
    void *aMethodArgs[] = {&aMethodArg};
    jinx::Jinx littleJinx("");                                                  // hello little jinx! we've been talking about you!

    // the method is defined as:
    // int aMethod(int v) {return v*2;}
    aMethod->call(&littleJinx, &aMethodRes, aMethodArgs);                       // this is how one would call a method through a wrapper. simplistic example but no reason it shouldn't be able to scale up
    EXPECT_EQ(aMethodRes, littleJinx.aMethod(aMethodArg));                      // for our test, we want both ways of calling the method to yield the same result
    EXPECT_THROW(aMethod->call(&static_cast<const jinx::Jinx&>(littleJinx), &aMethodRes, aMethodArgs), mc::const_corectness_error); // and const corectness is enforced too.


```

_Copyright (c) 2018 Bogdan Tudoran_
