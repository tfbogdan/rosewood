#pragma once

#include <cstdint>
#include <functional>
#include <string_view>

namespace mc {
    
    template <typename T>
    bool constexpr name_matches([[maybe_unused]]T val, std::string_view name) {
        return T::name == name;
    }

    template<typename MetaType>
    bool constexpr has_method([[maybe_unused]] MetaType metaType, std::string_view name) {
        using meta_type = MetaType;
        using methods_tuple_type = typename meta_type::methods;
        bool found_method = false;
        std::apply([&name, &found_method](auto ...meth) {
            found_method = found_method || ((name_matches(meth, name), ...));
        }, methods_tuple_type());
        return found_method;
    }

}
