#pragma once

#include <cstdint>
#include <functional>
#include <string_view>
#include <tuple>

namespace mc {
    
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
    struct Class {
        using descriptor = Descriptor;
        constexpr bool has_method(std::string_view methodName) const {
            using methods_tuple = typename descriptor::methods;
            return std::apply([methodName] (auto ...meth) {
                return (... || (meth.name == methodName));
            }, methods_tuple());
        }
    };
}
