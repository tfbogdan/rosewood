#pragma once

#include <mc/mc.hpp>
#include <array>
#include <functional>
#include <vector>

namespace mc {

    class DMetaDecl {
    public:
        virtual ~DMetaDecl() = 0;
        virtual std::string_view getName() const noexcept = 0;
    private:

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

    class DClass;

    class DNamespace : public DMetaDecl {
    public:
        virtual ~DNamespace() = 0;

        virtual const std::vector<const DNamespace*> getNamespaces() const noexcept = 0;
        virtual const std::vector<const DEnum*> getEnums() const noexcept = 0;
        virtual const std::vector<const DClass*> getClasses() const noexcept = 0;

    private:
    };

    class DClass : public DMetaDecl {
    public:
        virtual ~DClass() = 0;
        virtual bool hasMethod(std::string_view name) const noexcept = 0;
    private:
    };

    template <typename MetaClass>
    class DClassWrapper : public DClass {
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

    template<typename MetaNamespace>
    class DNamespaceWrapper : public DNamespace {
        using descriptor = MetaNamespace;
    public:

        inline virtual ~DNamespaceWrapper() = default;

        inline virtual std::string_view getName() const noexcept {
            return descriptor::name;
        }

        inline virtual const std::vector<const DNamespace*> getNamespaces() const noexcept {
            return namespaces;
        }

        inline virtual const std::vector<const DEnum*> getEnums() const noexcept {
            return enums;
        }

        inline virtual const std::vector<const DClass*> getClasses() const noexcept {
            return classes;
        }

        using child_namespaces_tuple = typename tuple_elements_wrapper<DNamespaceWrapper, typename descriptor::namespaces>::type;
        static const child_namespaces_tuple child_namespaces;

        using child_enums_tuple = typename tuple_elements_wrapper<DEnumWrapper, typename descriptor::enums>::type;
        static const child_enums_tuple child_enums;

        using child_classes_tuple = typename tuple_elements_wrapper<DClassWrapper, typename descriptor::classes>::type;
        static const child_classes_tuple child_classes;


        static const std::vector<const DNamespace*> namespaces;
        static const std::vector<const DEnum*> enums;
        static const std::vector<const DClass*> classes;
    };

    template<typename base_t, typename tuple_t>
    static std::vector<const base_t*> init_ptr_vector(const tuple_t& tuple) noexcept {
        if constexpr(std::tuple_size<tuple_t>::value == 0) {
            return std::vector<const base_t*>();
        } else {
            std::vector<const base_t*> result;
            result.reserve(std::tuple_size<tuple_t>::value);
            std::apply([&result](const auto &...elems) {
                ((result.push_back(&elems)),...);
            }, tuple);
            return result;
        }
    }

    template<typename MetaNamespace>
    const typename DNamespaceWrapper<MetaNamespace>::child_namespaces_tuple DNamespaceWrapper<MetaNamespace>::child_namespaces{};
    template<typename MetaNamespace>
    const typename DNamespaceWrapper<MetaNamespace>::child_enums_tuple DNamespaceWrapper<MetaNamespace>::child_enums{};
    template<typename MetaNamespace>
    const typename DNamespaceWrapper<MetaNamespace>::child_classes_tuple DNamespaceWrapper<MetaNamespace>::child_classes{};


    template<typename MetaNamespace>
    const std::vector<const DNamespace*> DNamespaceWrapper<MetaNamespace>::namespaces = init_ptr_vector<DNamespace>(DNamespaceWrapper<MetaNamespace>::child_namespaces);

    template<typename MetaNamespace>
    const std::vector<const DEnum*> DNamespaceWrapper<MetaNamespace>::enums = init_ptr_vector<DEnum>(DNamespaceWrapper<MetaNamespace>::child_enums);

    template<typename MetaNamespace>
    const std::vector<const DClass*> DNamespaceWrapper<MetaNamespace>::classes = init_ptr_vector<DClass>(DNamespaceWrapper<MetaNamespace>::child_classes);

}
