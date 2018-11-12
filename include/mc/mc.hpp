#pragma once

#include <cstdint>
#include <functional>
#include <string_view>
#include <tuple>

namespace mc {

    struct nil_t {};

    template <typename T>
    struct tuple_seeker;

    template <typename headT, typename ...remainingTypes>
    struct tuple_seeker<std::tuple<headT, remainingTypes...>> {
        using tail = std::tuple<remainingTypes...>;
    };

    template <typename headT>
    struct tuple_seeker<std::tuple<headT>> {
        using tail = std::tuple<>;
    };

    template <typename>
    struct tuple_tail;

    template <typename ...Tail>
    struct tuple_tail <std::tuple<Tail...>> {
        using type = std::tuple<>;
    };

    template <typename Head, typename ...Tail>
    struct tuple_tail <std::tuple<Head, Tail...>> {
        using type = std::tuple<Tail...>;
    };


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
    struct Enumerator {
        using descriptor = Descriptor;
        constexpr std::string_view get_name() const noexcept {
            return descriptor::name;
        }
    };

    template<typename Descriptor>
    struct Parameter {
        using descriptor = Descriptor;

        constexpr std::string_view get_name() const {
            return descriptor::name;
        }
    };

    template<typename Descriptor>
    struct Method {
        using descriptor = Descriptor;

        template <typename visitorT>
        void for_each_parameter(visitorT visitor) const {
            using param_tuple = typename descriptor::parameters;
            std::apply([visitor] (auto ...params){
                (visitor(params), ...);
            }, param_tuple());
        }

        constexpr int num_params() const noexcept {
            using parameters = typename descriptor::parameters;
            return std::tuple_size<parameters>::value;
        }

        template<unsigned index>
        constexpr auto get_param() const noexcept {
            using parameters = typename descriptor::parameters;
            using param = typename std::tuple_element<index, parameters>::type;
            return param();
        }
    };

    template<typename Descriptor>
    struct OverloadSet {
        using descriptor = Descriptor;
        constexpr std::string_view get_name() const {
            return descriptor::name;
        }

        template <typename visitorT>
        void for_each_overload(visitorT visitor) const noexcept {
            using overloads = typename descriptor::overloads;
            std::apply([visitor](auto ...overload){
                (visitor(overload), ...);
            }, overloads());
        }

        constexpr int num_overloads() const noexcept {
            using overloads = typename descriptor::overloads;
            return std::tuple_size<overloads>::value;
        }

        template<unsigned index>
        constexpr auto get_overload() const noexcept {
            using overloads = typename descriptor::overloads;
            using overload = typename std::tuple_element<index, overloads>::type;
            return overload();
        }
    };

    template<typename Descriptor>
    struct Operator {};

    template<typename Descriptor>
    struct ConstructorSet {};

    template<typename Descriptor>
    struct Field {};

    template <typename tupleT, typename pred>
    constexpr auto get_by_name(tupleT, pred p) noexcept {
        static_assert (std::tuple_size<tupleT>::value > 0, "no element found with ::name == pred::pred");
        using headT = typename std::tuple_element<0, tupleT>::type;
        if constexpr(pred::pred == headT::name) {
            return headT();
        } else {
            using tailT = typename tuple_seeker<tupleT>::tail;
            return get_by_name<typename tuple_seeker<tupleT>::tail>(tailT(), p);
        }
    }

    template<typename Descriptor>
    struct Class {

        using descriptor = Descriptor;
        constexpr bool has_overload_set(std::string_view Name) const noexcept {
            using methods_tuple = typename descriptor::methods;
            return std::apply([Name] (auto ...meth) {
                return (... || (meth.name == Name));
            }, methods_tuple());
        }

        template<typename visitorT>
        constexpr void visit_overload_sets(visitorT visitor) const noexcept {
            using overload_sets = typename descriptor::methods;
            std::apply([visitor](auto ...overloads) {
                (visitor(overloads), ...);
            }, overload_sets());
        }

        template<typename predicate>
        constexpr auto get_overload_set(predicate pred) const noexcept {
            using overloads_sets = typename descriptor::methods;
            return get_by_name(overloads_sets(), pred);
        }

    };



}
