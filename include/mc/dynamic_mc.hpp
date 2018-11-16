#pragma once

#include <mc/mc.hpp>
#include <array>
#include <functional>
#include <vector>
#include <unordered_map>
#include <stdexcept>

namespace mc {

    class const_corectness_error : public std::logic_error {
        using std::logic_error::logic_error;
    };

namespace detail {
	
template <typename sourceTupleT, typename baseType, template<typename> typename wrapperType>
struct range_model {
    using wrapped_elements_tuple = typename tuple_elements_wrapper<wrapperType, sourceTupleT>::type;
    static const wrapped_elements_tuple wrapped_elements;

    static constexpr int num_elements = std::tuple_size<sourceTupleT>::value;

    using base_array_t = std::array<const baseType*, num_elements>;
    static constexpr auto array_initializer() {
        return std::apply([](const auto& ...elems) {
            base_array_t result{};
            int index(0);
            ((result[index++] = &elems), ...);
            return result;
        }, wrapped_elements);
    }

    static constexpr base_array_t base_array = array_initializer();

    using map_type = std::unordered_map<std::string_view, const baseType*>;
    static constexpr auto map_initializer() {
        return std::apply([](const auto& ...elems) {
            map_type result;
            ((result[elems.getName()] = &elems), ...);
            return result;
        }, wrapped_elements);
    }

    static const map_type element_map;
};

template <typename sourceTupleT, typename baseType, template<typename> typename wrapperType>
const typename range_model<sourceTupleT, baseType, wrapperType>::wrapped_elements_tuple range_model<sourceTupleT, baseType, wrapperType>::wrapped_elements{};

template <typename sourceTupleT, typename baseType, template<typename> typename wrapperType>
const typename range_model<sourceTupleT, baseType, wrapperType>::map_type range_model<sourceTupleT, baseType, wrapperType>::element_map = range_model<sourceTupleT, baseType, wrapperType>::map_initializer();

}

    class DNamespace;

    class DMetaDecl {
    public:
        virtual ~DMetaDecl() = 0;
        virtual std::string_view getName() const noexcept = 0;
    };

    class DClass;
    class DClassContainer {
    public:
        virtual ~DClassContainer() = 0;
        virtual const DClass *findChildClass(std::string_view name) const noexcept(false) = 0;
    };

    template <typename Descriptor>
    class DClassWrapper;

    template <typename Descriptor>
    class DClassContainerWrapper {
    protected:
        using class_range = detail::range_model<typename Descriptor::classes, DClass, DClassWrapper>;
        static constexpr class_range classes = {};
    };

    class DEnum;
    class DEnumContainer {
    public:
        virtual ~DEnumContainer() = 0;
        virtual const DEnum *findChildEnum(std::string_view name) const noexcept(false) = 0;
    };


    class DEnumerator : public DMetaDecl {
    public:
        virtual ~DEnumerator() = 0;

    };

    class DEnum : public DMetaDecl {
    public:
        virtual ~DEnum() = 0;
    private:
    };

    template<typename MetaEnum>
    class DEnumWrapper : public DEnum {
        using descriptor = MetaEnum;
    public:
        inline virtual ~DEnumWrapper() = default;

        inline virtual std::string_view getName() const noexcept {
            return descriptor::name;
        }

    };

    class DClass;

    class DNamespace : public DMetaDecl, public DClassContainer, public DEnumContainer {
    public:
        virtual ~DNamespace() = 0;

        virtual const std::vector<const DNamespace*> getNamespaces() const noexcept = 0;
        virtual const std::vector<const DEnum*> getEnums() const noexcept = 0;
        virtual const std::vector<const DClass*> getClasses() const noexcept = 0;
        virtual const DNamespace *findChildNamespace(std::string_view name) const noexcept(false) = 0;
    private:
    };

    class DParameter : public DMetaDecl {
    public:
        virtual ~DParameter() = 0;
    };

    template <typename Descriptor>
    class DParameterWrapper : public DParameter {
    public:
        inline virtual ~DParameterWrapper() = default;
        inline virtual std::string_view getName() const noexcept final {
            return Descriptor::name;
        }
    };

    class DMethod : public DMetaDecl {
    public:
        virtual ~DMethod() = 0;

        /**
         * @brief call is a horribly unsafe API that can be ridiculously fast (for a runtime dispatch) provided that one has already checked the return/argument list types compatibility before calling
         * @param object pointer will be treated as an object of the class defining this method
         * @param retValAddr will be used to store the return value (if not void). This is completely unchecked! If a function is not returning void, then this value WILL be used even if nullptr!
         * @param args an array of arguments. Every argument is expected to be binary compatible to the argument list of the actual method wrapped by this instance. int to unsigned might work simply due to same binary representation ( sort of ) but a conversion from float to double will not happen through this API so use it with a lot of care (ideally hidden by a safer API)
         * noexcept guarantees simply cannot be made so expect this to throw anything at you as long as the wrapped method does so. nothing other than that, although that cannot be expressed in C++
         */
        virtual void call(const void *object, void *retValAddr, void **args) const = 0;
        virtual void call(void *object, void *retValAddr, void **args) const = 0;
    private:

    };

    template <typename Descriptor>
    class DMethodWrapper : public DMethod {
    public:
        inline virtual std::string_view getName() const noexcept final {
            return Descriptor::name;
        }

    private:
        using parameter_model = detail::range_model<typename Descriptor::parameters, DParameter, DParameterWrapper>;
        static constexpr parameter_model parameters {};

    public:
        virtual void call(const void *object, void *retValAddr, void **args) const final {
            if constexpr (Descriptor::is_const) {
                Descriptor::fastcall(object, retValAddr, args);
            } else {
                throw const_corectness_error("non const method called on const object");
            }
        }

        inline virtual void call(void *object, void *retValAddr, void **args) const final {
            Descriptor::fastcall(object, retValAddr, args);
        }

    private:
    };

    class DOverloadSet : public DMetaDecl {
    public:
        virtual ~DOverloadSet() = 0;
        virtual std::vector<const DMethod*> getMethods() const noexcept = 0;
    };

    template <typename Descriptor>
    class DOverloadSetWrapper : public DOverloadSet {
    public:
        inline virtual ~DOverloadSetWrapper() = default;
        inline virtual std::string_view getName() const noexcept final {
            return Descriptor::name;
        }

        inline virtual std::vector<const DMethod*> getMethods() const noexcept final {
            return std::vector<const DMethod*>(overloads.base_array.begin(), overloads.base_array.end());
        }

    private:
        using overload_model =detail::range_model<typename Descriptor::overloads, DMethod, DMethodWrapper>;
        static constexpr overload_model overloads {};
    };

    class DField : public DMetaDecl {
    public:
        virtual ~DField() = 0;
    };

    template <typename Descriptor>
    class DFieldWrapper : public DField {
    public:
        inline virtual ~DFieldWrapper() = default;
        inline virtual std::string_view getName() const noexcept final {
            return Descriptor::name;
        }
    };

    class DClass : public DMetaDecl, public DClassContainer, public DEnumContainer {
    public:
        virtual ~DClass() = 0;
        virtual bool hasMethod(std::string_view name) const noexcept = 0;
        virtual const DOverloadSet *findOverloadSet(std::string_view name) const noexcept(false) = 0;
        virtual const DField *findField(std::string_view name) const noexcept(false) = 0;
    private:
    };

    template <typename MetaClass>
    class DClassWrapper : public DClass {
    public:
        using descriptor = MetaClass;

        inline virtual ~DClassWrapper() = default;

        inline virtual std::string_view getName() const noexcept final {
            return descriptor::name;
        }

        inline virtual bool hasMethod(std::string_view name) const noexcept final {
            descriptor desc;
            return desc.has_overload_set(name);
        }

        inline virtual const DClass *findChildClass(std::string_view name) const noexcept(false) final {
            return  classes.element_map.at(name);
        }

        inline virtual const DEnum *findChildEnum(std::string_view name) const noexcept(false) final {
            return enums.element_map.at(name);
        }

        inline virtual const DOverloadSet *findOverloadSet(std::string_view name) const noexcept(false) final {
            return overload_sets.element_map.at(name);
        }

        inline virtual const DField *findField(std::string_view name) const noexcept(false) final {
            return fields.element_map.at(name);
        }

    private:
        using enum_range = detail::range_model<typename MetaClass::enums, DEnum, DEnumWrapper>;
        static constexpr enum_range enums = {};

		template<typename T>
		using disambiguator = DClassWrapper<T>;

        using class_range = detail::range_model<typename MetaClass::classes, DClass, disambiguator>;
        static constexpr class_range classes = {};

        using overload_set_model = detail::range_model<typename descriptor::overload_sets, DOverloadSet, DOverloadSetWrapper>;
        static constexpr overload_set_model overload_sets {};

         using field_model = detail::range_model<typename descriptor::fields, DField, DFieldWrapper>;
        static constexpr field_model  fields {};
    };


    template<typename MetaNamespace>
    class DNamespaceWrapper : public DNamespace {
        using descriptor = MetaNamespace;
    public:

        inline virtual ~DNamespaceWrapper() = default;

        inline virtual std::string_view getName() const noexcept {
            return descriptor::name;
        }

        inline virtual const std::vector<const DNamespace*> getNamespaces() const noexcept override final {
            return std::vector<const DNamespace*>(namespaces.base_array.begin(), namespaces.base_array.end());
        }

        inline virtual const std::vector<const DClass*> getClasses() const noexcept override final {
            return std::vector<const DClass*>(
                        classes.base_array.begin(),
                        classes.base_array.end()
            );
        }

        inline virtual const std::vector<const DEnum*> getEnums() const noexcept override final {
            return std::vector<const DEnum*>(
                        enums.base_array.begin(),
                        enums.base_array.end()
            );
        }

        inline virtual const DNamespace *findChildNamespace(std::string_view name) const noexcept(false) override final {
            return namespaces.element_map.at(name);
        }

        inline virtual const DClass *findChildClass(std::string_view name) const noexcept(false) override final {
            return classes.element_map.at(name);
        }

        inline virtual const DEnum *findChildEnum(std::string_view name) const noexcept(false) override final {
            return enums.element_map.at(name);
        }

    protected:
        using enum_range = detail::range_model<typename MetaNamespace::enums, DEnum, DEnumWrapper>;
        static constexpr enum_range enums = {};

        using class_range = detail::range_model<typename MetaNamespace::classes, DClass, DClassWrapper>;
        static constexpr class_range classes = {};

		template <typename T>
		using disambiguator = DNamespaceWrapper<T>;
        using namespaces_model = detail::range_model<typename descriptor::namespaces, DNamespace, disambiguator>;
        static constexpr namespaces_model namespaces {};
    };

}
