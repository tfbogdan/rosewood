#pragma once

#include "mc.hpp"
#include "dynamic_mc.hpp"

#include <unordered_map>
#include <string_view>
#include <tuple>

namespace mc {

/**
 *  @class StaticIndex is a utlity that makes searching for declarations easier.
 * Declarations are essentially stored in a tree
 * so this class flattens that structure and maintains a bunch of data structures for fast lookups (has tables?)
 */
template <typename ...WrappedTypes>
class StaticIndex {


private:

    // for starteres, all declarations need to be stored somewhere.
    // of course, all members of this tuple will have their nested types contained within so for us storing them is enough
    static const std::tuple<WrappedTypes...> toplevel_types;

    static auto init_name_lookup() {
        std::unordered_map<std::string_view, const mc::DMetaDecl*> result;
        std::apply([&result] (const auto &...decls) {
            // once the top level declarations are added to the index, then
            ((result[decls.getQualifiedName()] = &decls), ...);
        }, toplevel_types);
        return result;
    }

    // when an entity is searched for by it's full name than nothing beats a hash table
    static const std::unordered_map<std::string_view, const mc::DMetaDecl*> name_lookup;
};

template <typename ...WrappedTypes>
const std::tuple<WrappedTypes...> StaticIndex<WrappedTypes...>::toplevel_types;

template <typename ...WrappedTypes>
const std::unordered_map<std::string_view, const mc::DMetaDecl*> StaticIndex<WrappedTypes...>::name_lookup = StaticIndex<WrappedTypes...>::init_name_lookup();

}
