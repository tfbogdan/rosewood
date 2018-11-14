#pragma once

#include <mc/mc.hpp>
#include <array>
#include <functional>
#include <vector>
#include <unordered_map>

namespace mc {

namespace detail {

template <typename sourceTupleT, typename baseType, template<typename> typename wrapperType>
struct range_model {
    using wrapped_elements_tuple = typename tuple_elements_wrapper<wrapperType, sourceTupleT>::type;
    static const wrapped_elements_tuple wrapped_elements;

    static constexpr int num_elements = std::tuple_size<sourceTupleT>::value;

    using base_array_t = std::array<const baseType*, num_elements>;
    // the array initializer
    static constexpr auto array_initializer = [] () constexpr {
        if constexpr (std::tuple_size<sourceTupleT>::value > 0) {
            return std::apply([](const auto& ...elems) {
                return base_array_t{
                    ((&elems), ...)
                };
            }, wrapped_elements);
        } else {
            return base_array_t{};
        }
    };

    static constexpr base_array_t base_array = array_initializer();

    using map_type = std::unordered_map<std::string_view, const baseType*>;
    static constexpr auto map_initializer = [] () constexpr {
        if constexpr (std::tuple_size<sourceTupleT>::value > 0) {
            return std::apply([](const auto& ...elems) {
                return map_type{
                    (std::pair(elems.getName(), &elems), ...)
                };
            }, wrapped_elements);
        } else {
            return map_type{};
        }
    };

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

    template <typename Descriptor>
    class DEnumContainerWrapper : public DEnumContainer {
    protected:
        using enum_range = detail::range_model<typename Descriptor::enums, DEnum, DEnumWrapper>;
        static constexpr enum_range enums = {};
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

    class DClass : public DMetaDecl, public DClassContainer, public DEnumContainer {
    public:
        virtual ~DClass() = 0;
        virtual bool hasMethod(std::string_view name) const noexcept = 0;
    private:
    };

    template <typename MetaClass>
    class DClassWrapper : public DClass, public DEnumContainerWrapper<MetaClass>, public DClassContainerWrapper<MetaClass> {
    public:
        using descriptor = MetaClass;

        inline virtual ~DClassWrapper() = default;

        inline virtual std::string_view getName() const noexcept {
            return descriptor::name;
        }

        inline virtual bool hasMethod(std::string_view name) const noexcept {
            descriptor desc;
            return desc.has_overload_set(name);
        }

        inline virtual const DClass *findChildClass(std::string_view name) const noexcept(false) override final {
            return  DClassContainerWrapper<MetaClass>::classes.element_map.at(name);
        }

        inline virtual const DEnum *findChildEnum(std::string_view name) const noexcept(false) override final {
            return DEnumContainerWrapper<MetaClass>::enums.element_map.at(name);
        }
    };


    template<typename MetaNamespace>
    class DNamespaceWrapper : public DNamespace, public DEnumContainerWrapper<MetaNamespace>, public DClassContainerWrapper<MetaNamespace> {
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
                        DClassContainerWrapper<MetaNamespace>::classes.base_array.begin(),
                        DClassContainerWrapper<MetaNamespace>::classes.base_array.end()
            );
        }

        inline virtual const std::vector<const DEnum*> getEnums() const noexcept override final {
            return std::vector<const DEnum*>(
                        DEnumContainerWrapper<MetaNamespace>::enums.base_array.begin(),
                        DEnumContainerWrapper<MetaNamespace>::enums.base_array.end()
            );
        }

        inline virtual const DNamespace *findChildNamespace(std::string_view name) const noexcept(false) override final {
            return namespaces.element_map.at(name);
        }

        inline virtual const DClass *findChildClass(std::string_view name) const noexcept(false) override final {
            return DClassContainerWrapper<MetaNamespace>::classes.element_map.at(name);
        }

        inline virtual const DEnum *findChildEnum(std::string_view name) const noexcept(false) override final {
            return DEnumContainerWrapper<MetaNamespace>::enums.element_map.at(name);
        }

    protected:
        using namespaces_model = detail::range_model<typename descriptor::namespaces, DNamespace, DNamespaceWrapper>;
        static constexpr namespaces_model namespaces {};
    };

}
