#pragma once

#include <cstdint>
#include <functional>
#include <string_view>
#include <algorithm>
#include <tuple>
#include <type_traits>

namespace rosewood {
    struct nil_t {};

    template <typename t1, typename t2>
    struct tuple_cat_static;

    template<typename ...tuples>
    struct tuple_type_cat {
        static constexpr auto apply_cat = [] () {
            return std::apply([](auto ...tuple_types){
                return std::tuple_cat(((tuple_types),...));
            }, std::tuple<tuples...>{});
        };
        using type = decltype(apply_cat());
    };

    template <typename T>
    constexpr decltype (auto) construct() noexcept {
        return T();
    }

    template <template<typename> typename wrapping_type, typename Tuple>
    struct tuple_elements_wrapper;

    template <template<typename> typename wrapping_type>
    struct tuple_elements_wrapper<wrapping_type, std::tuple<>> {
        using type =std::tuple<>;
    };

    template <template<typename> typename wrapping_type, typename TupleHead, typename ...TupleTypes>
    struct tuple_elements_wrapper<wrapping_type, std::tuple<TupleHead, TupleTypes...>> {
        using type = decltype(
            std::tuple_cat(
                construct<std::tuple<wrapping_type<TupleHead>>>(),
                construct<typename tuple_elements_wrapper<wrapping_type, std::tuple<TupleTypes...>>::type>()
            )
        );
    };


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
            return std::apply([value] (auto ...enums) {
                return (... || (enums.value == value));
            }, descriptor::enumerators);
        }

        template <typename visitorT>
        constexpr void for_each_enumerator(visitorT visitor) const {
            std::apply([visitor] (auto ...enums) {
                (visitor(enums), ...);
            }, descriptor::enumerators);
        }
    };



    template<typename EnumType>
    struct Enumerator {
        constexpr std::string_view get_name() const noexcept {
            return name;
        }

        constexpr auto get_value() const noexcept {
            return value;
        }
        EnumType value;
        std::string_view name;
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

	template <typename ArgType> 
	struct FunctionParameter {
		using type_t = ArgType;

		std::string_view name;
	};

	template<typename ClassType, typename ReturnType, bool isConst, bool isNoExcept, typename ...ArgTypes>
	struct MethodDeclaration {
		using class_type = ClassType;
		using return_type = ReturnType;
		static constexpr bool is_const = isConst;
		static constexpr bool is_noexcept = isNoExcept;

		using arg_types = typename tuple_elements_wrapper<FunctionParameter, std::tuple<ArgTypes...>>::type;

		std::string_view	name;
		arg_types			args;
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
            using methods_tuple = typename descriptor::overload_sets;
            return std::apply([Name] (auto ...meth) {
                return (... || (meth.name == Name));
            }, methods_tuple());
        }

        template<typename visitorT>
        constexpr void visit_overload_sets(visitorT visitor) const noexcept {
            using overload_sets = typename descriptor::overload_sets;
            std::apply([visitor](auto ...overloads) {
                (visitor(overloads), ...);
            }, overload_sets());
        }

        template<typename predicate>
        constexpr auto get_overload_set(predicate pred) const noexcept {
            using overloads_sets = typename descriptor::overload_sets;
            return get_by_name(overloads_sets(), pred);
        }

    };


    template<typename T>
    struct is_enum {
        static constexpr bool value = false;
    };

    template<typename DescType>
    struct is_enum<Enum<DescType>> {
        static constexpr bool value = true;
    };

    template<typename T>
    struct is_class {
        static constexpr bool value = false;
    };

    template<typename DescType>
    struct is_class<Class<DescType>> {
        static constexpr bool value = true;
    };


    template<typename T>
    struct is_namespace {
        static constexpr bool value = false;
    };

    template<typename DescType>
    struct is_namespace<Namespace<DescType>> {
        static constexpr bool value = true;
    };
}
