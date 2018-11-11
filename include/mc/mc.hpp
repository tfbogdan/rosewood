#pragma once

#include <cstdint>
#include <functional>
#include <string_view>
#include <tuple>

namespace mc {

    template<typename T>
    struct meta;
    
    template<typename Descriptor>
    struct Module {};

    template<typename Descriptor>
    struct Namespace {};

    template<typename Descriptor>
    struct Enum {
        using descriptor = Descriptor;

        constexpr bool in_range(int value) const {
            using enumerators_tuple = typename descriptor::enumerators;
            return std::apply([value] (auto ...enums) {
                return (... || (enums.value == value));
            }, enumerators_tuple());
        }

        template <typename visitorT>
        constexpr void for_each_enumerator(visitorT visitor) const {
            using enumerators_tuple = typename descriptor::enumerators;
            std::apply([visitor] (auto ...enums) {
                (visitor(enums), ...);
            }, enumerators_tuple());
        }
    };

    template<typename Descriptor>
    struct Enumerator {};

    template<typename Descriptor>
    struct Parameter {};

    template<typename Descriptor>
    struct Method {
        using descriptor = Descriptor;

        template <typename ...Args>
        constexpr auto operator()(Args&& ...args) {

        }
    };

    template<typename Descriptor>
    struct OverloadSet {};

    template<typename Descriptor>
    struct Operator {};

    template<typename Descriptor>
    struct ConstructorSet {};

    template<typename Descriptor>
    struct Field {};

    template<typename Descriptor>
    struct Class {
        using descriptor = Descriptor;
        constexpr bool has_method(std::string_view methodName) const {
            using methods_tuple = typename descriptor::methods;
            return std::apply([methodName] (auto ...meth) {
                return (... || (meth.name == methodName));
            }, methods_tuple());
        }
    };


    class DynamicClass {
    public:
        virtual ~DynamicClass() = 0;
        virtual bool hasMethod(std::string_view name) const = 0;
        // virtual void call(std::string_view method, const void *obj, int argc, void **argv) const = 0;
        // virtual void call(std::string_view method, void *obj, int argc, void **argv) = 0;
    private:
    };
    DynamicClass::~DynamicClass() = default;

    template <typename WrappedType>
    class DynamicClassWrapper : public DynamicClass, public mc::meta<WrappedType>{
        using descriptor = mc::meta<WrappedType>;

        inline virtual ~DynamicClassWrapper() = default;
        inline virtual bool hasMethod(std::string_view name) const {
            return descriptor::has_method(name);
        }

    };

}
