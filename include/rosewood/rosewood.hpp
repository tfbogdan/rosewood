#pragma once

#include <cstdint>
#include <functional>
#include <string_view>
#include <algorithm>
#include <tuple>
#include <type_traits>

namespace rosewood {
    struct nil_t {};

    template <template<typename> typename wrapping_type, typename ...Ts>
    struct tuple_elements_wrapper;

    template <template<typename> typename wrapping_type, typename ...WrappedTupleTypes>
    struct tuple_elements_wrapper<wrapping_type, std::tuple<WrappedTupleTypes...>, std::tuple<>> {
        using type = std::tuple<WrappedTupleTypes...>;
    };

    template <template<typename> typename wrapping_type, typename ...WrappedTupleTypes, typename Head, typename ...UnwrappedTypes>
    struct tuple_elements_wrapper<wrapping_type, std::tuple<WrappedTupleTypes...>, std::tuple<Head, UnwrappedTypes...>> {
        using type = typename tuple_elements_wrapper<wrapping_type, std::tuple<WrappedTupleTypes..., wrapping_type<Head>>, std::tuple<UnwrappedTypes...>>::type;
    };

    template <template<typename> typename wrapping_type, typename ...Types>
    struct arguments_wrapper;

    template <template<typename> typename wrapping_type, typename ...TupleTypes>
    struct arguments_wrapper<wrapping_type, std::tuple<TupleTypes...>> {
        using type = std::tuple<TupleTypes...>;
    };

    template <template<typename> typename wrapping_type, typename Head, typename ...Types, typename ...TupleTypes>
    struct arguments_wrapper<wrapping_type, std::tuple<TupleTypes...>, Head, Types...> {
        using type = typename arguments_wrapper<wrapping_type, std::tuple<TupleTypes..., wrapping_type<Head>>, Types...>::type;
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

    template <typename ArgType>
    struct FunctionParameter {
        using type_t = ArgType;

        constexpr FunctionParameter(std::string_view argName, bool hasDefaultValue, int pos) noexcept
            : name(argName),
              isDefaulted(hasDefaultValue),
              arg_pos(pos) {}

        constexpr FunctionParameter() noexcept = default;
        constexpr FunctionParameter(const FunctionParameter&) noexcept = default;
        constexpr FunctionParameter(FunctionParameter&&) noexcept = default;

        std::string_view name = "";
        bool isDefaulted = false;
        int arg_pos = 0;

        // Is a reference the correct type to work with here?
        // certainly needs a bit of looking into
        void *eraseType(type_t &t) const noexcept {
            return static_cast<void*>(&t);
        }

        decltype(auto) narrowType(void *ptr) const noexcept {
            return static_cast<type_t>
                    (*reinterpret_cast<typename std::remove_reference<type_t>::type*>(ptr));
        }
    };

    template <typename T>
    struct ReturnTypeHandler {
        using type_t = T;
        static auto narrowType(void *ptr) noexcept {
            return reinterpret_cast<
                    typename std::remove_const<
                        typename std::remove_reference<
                            typename std::remove_const<type_t>::type
                        >::type
                    >::type*
                 >(ptr);
        }
    };

#ifdef _MSC_VER	// MSVC strips constness when deducing function parameter types.
    template <typename ArgType>
    struct FunctionParameter<const ArgType> : public FunctionParameter<ArgType> {
        using impl_t = FunctionParameter<ArgType>;
        using type_t = FunctionParameter<const ArgType>;

        constexpr type_t(std::string_view argName, bool hasDefaultValue) noexcept
            :impl_t(argName, hasDefaultValue) {}
    };

#endif // _MSC_VER

    template <typename ClassType, typename ReturnType, bool Const, bool NoExcept, typename ...Args>
    struct MethodTypeCompositor;

    template <typename ClassType, typename ReturnType, typename ...Args>
    struct MethodTypeCompositor<ClassType, ReturnType, true, true, Args...> {
        using method_type = ReturnType (ClassType::*)(Args...) const noexcept;
        using object_type = const ClassType;
    };

    template <typename ClassType, typename ReturnType, typename ...Args>
    struct MethodTypeCompositor<ClassType, ReturnType, true, false, Args...> {
        using method_type = ReturnType (ClassType::*)(Args...) const;
        using object_type = const ClassType;
    };

    template <typename ClassType, typename ReturnType, typename ...Args>
    struct MethodTypeCompositor<ClassType, ReturnType, false, true, Args...> {
        using method_type = ReturnType (ClassType::*)(Args...) noexcept;
        using object_type = ClassType;
    };

    template <typename ClassType, typename ReturnType, typename ...Args>
    struct MethodTypeCompositor<ClassType, ReturnType, false, false, Args...> {
        using method_type = ReturnType (ClassType::*)(Args...);
        using object_type = ClassType;
    };

    template<typename ClassType, typename ReturnType, bool Const, bool NoExcept, typename ...ArgTypes>
    struct MethodDeclaration {
        using class_type = ClassType;
        using return_type = ReturnType;
        using arg_types = typename arguments_wrapper<FunctionParameter, std::tuple<>, ArgTypes...>::type;
        using type_decompositor = MethodTypeCompositor<ClassType, ReturnType, Const, NoExcept, ArgTypes...>;
        using method_type = typename type_decompositor::method_type;
        static constexpr bool is_const = Const;
        static constexpr bool is_noexcept = NoExcept;
        static constexpr std::size_t num_args = std::tuple_size<arg_types>::value;

        constexpr MethodDeclaration(method_type methodPtr, std::string_view method_name, arg_types &&arguments) noexcept
            : method_ptr(methodPtr),
              name(method_name),
              args(std::move(arguments)) {}

        constexpr bool isConst() const noexcept {
            return is_const;
        }

        constexpr bool isNoExcept() const noexcept {
            return is_noexcept;
        }

        method_type         method_ptr;
        std::string_view	name;
        arg_types			args;


        inline void invoke(void* object, void* ret, void **pArgs) const {
            invoke_impl (object, ret, pArgs);
        }

        inline void invoke(const void* object, void* ret, void **pArgs) const {
            static_assert (is_const, "");
            invoke_impl (const_cast<void*>(object), ret, pArgs);
        }

        inline void invoke_noexcept(void* object, void* ret, void **pArgs) const noexcept {
            static_assert (is_noexcept, "");
            invoke_impl (object, ret, pArgs);
        }

        inline void invoke_noexcept(const void* object, void* ret, void **pArgs) const noexcept {
            static_assert (is_const, "");
            static_assert (is_noexcept, "");
            invoke_impl (const_cast<void*>(object), ret, pArgs);
        }

        constexpr bool is_called(std::string_view nm) const noexcept {
            return name == nm;
        }

    private:

        inline void invoke_impl(void* object, void* ret, void** pArgs) const {
            using object_type = typename type_decompositor::object_type;
            auto obj = reinterpret_cast<object_type*>(object);

            std::apply([this, pArgs, ret, obj](auto& ...arg){
                if constexpr (std::is_void<return_type>::value) {
                    (obj->*method_ptr)(arg.narrowType(pArgs[arg.arg_pos]) ...);
                } else {
                    *ReturnTypeHandler<return_type>::narrowType(ret)
                            = (obj->*method_ptr)(arg.narrowType(pArgs[arg.arg_pos]) ...);
                }
            }, args);
        }
    };

    template <typename ClassType, bool NoExcept, typename ...ArgTypes>
    struct ConstructorDeclaration {
        using class_type = ClassType;
        using arg_types = typename arguments_wrapper<FunctionParameter, std::tuple<>, ArgTypes...>::type;
        static constexpr unsigned num_args = std::tuple_size<arg_types>::value;
        static constexpr bool is_noexcept = NoExcept;

        arg_types arguments;

        constexpr ConstructorDeclaration(arg_types&& args) noexcept
            :arguments(args) {}

        inline void invoke(void* object, void **pArgs) const {
            invoke_impl (object, pArgs);
        }

        inline void invoke_noexcept(void* object, void **pArgs) const noexcept {
            static_assert (is_noexcept, "");
            invoke_impl (object, pArgs);
        }

    private:

        void invoke_impl(void* addr, void** pArgs) const {
            std::apply([this, pArgs, addr] (auto& ...arg) {
                new (addr) ClassType(arg.narrowType(pArgs[arg.arg_pos]) ...);
            }, arguments);
        }
    };

    template<typename ClassType, typename ReturnType, typename ...ArgTypes>
    MethodDeclaration(ReturnType(ClassType::*)(ArgTypes...) const noexcept, std::string_view, typename MethodDeclaration<ClassType, ReturnType, true, true, ArgTypes...>::arg_types &&) -> MethodDeclaration<ClassType, ReturnType, true, true, ArgTypes...>;
    template<typename ClassType, typename ReturnType, typename ...ArgTypes>
    MethodDeclaration(ReturnType(ClassType::*)(ArgTypes...) noexcept, std::string_view, typename MethodDeclaration<ClassType, ReturnType, false, true, ArgTypes...>::arg_types&&)->MethodDeclaration<ClassType, ReturnType, false, true, ArgTypes...>;
    template<typename ClassType, typename ReturnType, typename ...ArgTypes>
    MethodDeclaration(ReturnType(ClassType::*)(ArgTypes...) const, std::string_view, typename MethodDeclaration<ClassType, ReturnType, true, false, ArgTypes...>::arg_types&&)->MethodDeclaration<ClassType, ReturnType, true, false, ArgTypes... >;
    template<typename ClassType, typename ReturnType, typename ...ArgTypes>
    MethodDeclaration(ReturnType(ClassType::*)(ArgTypes...), std::string_view, typename MethodDeclaration<ClassType, ReturnType, false, false, ArgTypes...>::arg_types&&)->MethodDeclaration<ClassType, ReturnType, false, false, ArgTypes...>;


    template <typename Type, typename ClassType>
    struct FieldDeclaration {
        using type_t = Type;
        using address_type = Type ClassType::*;

        constexpr FieldDeclaration(std::string_view nm, address_type addr)
            :name(nm),
            address(addr) {}

        std::string_view name;
        address_type address;

        void assign_copy(void *obj, void *from) const {
            auto *object = reinterpret_cast<ClassType*>(obj);
            (object->*address) = *reinterpret_cast<type_t*>(from);
        }

        void assign_move(void *obj, void *from) const {
            auto *object = reinterpret_cast<ClassType*>(obj);
            (object->*address) = std::move(*reinterpret_cast<type_t*>(from));
        }

    };

    template<typename Descriptor>
    struct Class {

        using descriptor = Descriptor;

        constexpr bool has_method(std::string_view Name) const noexcept {
            return std::apply([Name] (auto ...meth) {
                return (... || (meth.name == Name));
            }, Descriptor::methods);
        }

        template<typename visitorT>
        constexpr void visit_methods(visitorT visitor) const noexcept {
            std::apply([visitor](auto ...methods) {
                (visitor(methods), ...);
            }, Descriptor::methods);
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
