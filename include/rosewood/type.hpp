#pragma once

#include <string_view>

namespace rosewood {


template <typename theT>
struct Type {
    using type = theT;

    constexpr Type(std::string_view Name, std::string_view CanonicalName, std::string_view AtomicName)
        :name(Name),
        canonical_name(CanonicalName),
        atomic_name(AtomicName) {}

    std::string_view name;
    std::string_view canonical_name;
    std::string_view atomic_name;
};

}
