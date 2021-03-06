#pragma once

#include "rosewood.hpp"
#include "runtime.hpp"

#include <unordered_map>
#include <string_view>
#include <tuple>

namespace rosewood {

    class Index : public DeclarationContext {
    public:

    };
/**
 *  @class StaticIndex is a utlity that makes searching for declarations easier.
 * Declarations are essentially stored in a tree
 * so this class flattens that structure and maintains a bunch of data structures for fast lookups (has tables?)
 */
template <typename ...WrappedTypes>
class StaticIndex : public Index {
public:
    StaticIndex() {
        init_toplevel_lookups();
    }

    const Declaration *getDeclaration(std::string_view name) const noexcept final {
        if (auto res = toplevel_declarations.find(name); res != toplevel_declarations.end()) {
            return res->second.get();
        }
        return nullptr;
    }

private:

    template<unsigned tIdx = 0>
    void init_toplevel_lookups() {
        using tuple_type = std::tuple<WrappedTypes...>;

        if constexpr (tIdx < std::tuple_size<tuple_type>::value) {
            using current_type = typename std::tuple_element<tIdx, tuple_type>::type;
            using enums_type = typename current_type::enums;
            using classes_type = typename current_type::classes;
            using namespaces_type = typename current_type::namespaces;
            constexpr enums_type enums;
            constexpr classes_type classes;
            constexpr namespaces_type namespaces;

            std::apply([this] (auto &&...enums) {
                ((toplevel_declarations[enums.name] = std::move(std::make_unique<DEnumWrapper>((enums, this)))), ...);
            }, enums);

            std::apply([this] (auto &&...classes) {
                ((toplevel_declarations[classes.name] = std::move(std::make_unique<DClassWrapper>((classes, this)))), ...);
            }, classes);

            std::apply([this] (auto &&...nmspcs) {
                ((toplevel_declarations[nmspcs.name] = rosewood::makeNamespace(nmspcs, this)), ...);
            }, namespaces);

            init_toplevel_lookups<tIdx+1>();
        }
    }

    // for starteres, all declarations need to be stored somewhere.
    // of course, all members of this tuple will have their nested types contained within so for us storing them is enough
    // static const std::tuple<WrappedTypes...> toplevel_types;
    // using topleveltypes_t = typename rosewood::arguments_wrapper<rosewood::DNamespaceWrapper, std::tuple<>, std::tuple<WrappedTypes...>>::type;

    // topleveltypes_t topleveltypes;
    // when an entity is searched for by it's full name than nothing beats a hash table
    std::unordered_map<std::string_view, std::unique_ptr<rosewood::Declaration>> toplevel_declarations;
};

// template <typename ...WrappedTypes>
// const std::tuple<WrappedTypes...> StaticIndex<WrappedTypes...>::toplevel_types;

// template <typename ...WrappedTypes>
// const std::unordered_map<std::string_view, const rosewood::DMetaDecl*> StaticIndex<WrappedTypes...>::name_lookup = StaticIndex<WrappedTypes...>::init_name_lookup();

}
