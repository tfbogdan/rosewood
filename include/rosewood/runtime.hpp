#pragma once

#include <rosewood/rosewood.hpp>
#include <array>
#include <functional>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <memory>

namespace rosewood {

    class const_corectness_error : public std::logic_error {
        using std::logic_error::logic_error;
    };

    class DeclarationContext;

    class DType {
    public:
        virtual ~DType() = 0;
        /**
         * @brief getCanonicalName provides the canonical name of this type. eg, for std::string view it is std::basic_string<char>
         * @return
         */
        virtual std::string_view getCanonicalName() const noexcept = 0;
        /**
         * @brief getName returns the readable name of a type, exactly as it was typed by it's user.
         * @return
         */
        virtual std::string_view getName() const noexcept = 0;
        /**
         * @brief getAtomicName returns the name of the underlying type after decomosition: dropping of qualifiers and pointers and references. For const std:string&, that would be std::string
         * @return
         */
        virtual std::string_view getAtomicName() const noexcept = 0;
    };

    template <typename UnderlyingType>
    class DTypeWrapper : public DType {
    public:
        inline virtual ~DTypeWrapper() = default;

        inline DTypeWrapper(UnderlyingType type)
            :typeInstance(type) {}

        inline virtual std::string_view getCanonicalName() const noexcept final {
            return typeInstance.canonical_name;
        }

        inline virtual std::string_view getName() const noexcept final {
            return typeInstance.name;
        }

        inline virtual std::string_view getAtomicName() const noexcept final {
            return typeInstance.atomic_name;
        }

    private:
        const UnderlyingType typeInstance;
    };

    class DTypedDeclaration {
    public:
        virtual ~DTypedDeclaration() = 0;
        virtual const DType *getType() const noexcept = 0;
    };


    class DNamespace;
    class Class;
    class DEnum;
    class DMethod;
    class DEnumerator;
    class DField;
    class TypeDeclaration;

    class Declaration {
    public:
        explicit Declaration(const DeclarationContext *Parent);

        virtual ~Declaration() = 0;
        virtual std::string_view getName() const noexcept = 0;
        // virtual std::string_view getQualifiedName() const noexcept = 0;

        virtual const DNamespace *asNamespace() const noexcept;
        virtual const Class *asClass() const noexcept;
        virtual const DEnum *asEnum() const noexcept;
        virtual const DEnumerator *asEnumerator() const noexcept;
        virtual const DMethod *asMethod() const noexcept;
        virtual const DField *asField() const noexcept;
        virtual const DeclarationContext *asDeclContext() const noexcept;
        virtual const TypeDeclaration *asTypeDeclaration() const noexcept;

        virtual const DeclarationContext *parent() const noexcept;

    private:
        const DeclarationContext *parentP;
    };

    class TypeDeclaration : public Declaration {
    public:
        using Declaration::Declaration;

        virtual ~TypeDeclaration() = 0;

        const TypeDeclaration *asTypeDeclaration() const noexcept final;
    public:
    };

    class DeclarationContext {
    public:
        virtual ~DeclarationContext() = 0;
        virtual const Declaration *getDeclaration(std::string_view name) const noexcept = 0;
    };

    class Class;

    template <typename Descriptor>
    class DClassWrapper;

    class DEnum;
    class DEnumerator : public Declaration {
    public:
        virtual ~DEnumerator() = 0;
        virtual long long getValue() const noexcept = 0;
    };

    template <typename Descriptor>
    class DEnumeratorWrapper : public DEnumerator {
    public:
        DEnumeratorWrapper(const Descriptor &d) : descriptor(d) {}

        inline virtual std::string_view getName() const noexcept final {
            return descriptor.name;
        }

        inline virtual long long getValue() const noexcept final {
            return descriptor.value;
        }

        const Descriptor descriptor;
    };

    class DEnum : public TypeDeclaration {
    public:
        using TypeDeclaration::TypeDeclaration;

        virtual ~DEnum() = 0;
        virtual const DEnum *asEnum() const noexcept final;
        virtual std::vector<const DEnumerator*> getEnumerators() const noexcept = 0;
    private:
    };

    template<typename MetaEnum>
    class DEnumWrapper : public DEnum {
        using descriptor = MetaEnum;
    public:
        DEnumWrapper(const MetaEnum &me, const DeclarationContext* parent)
            : DEnum(parent) {
            initEnumeratorsVector();
        }

        inline virtual ~DEnumWrapper() = default;

        inline virtual std::string_view getName() const noexcept {
            return descriptor::name;
        }

        inline virtual std::vector<const DEnumerator*> getEnumerators() const noexcept final {
            std::vector<const DEnumerator*> result; result.reserve(enumerators.size());
            for (const auto &en : enumerators) {
                result.emplace_back(&en);
            }
            return result;
        }

    private:
        void initEnumeratorsVector() {
            enumerators.reserve(MetaEnum::enumerators.size());
            for (const auto &en: MetaEnum::enumerators) {
                enumerators.emplace_back(en);
            }
        }
        std::vector<DEnumeratorWrapper<typename descriptor::enumerator_type>> enumerators;
    };

    template <typename T>
    DEnumWrapper(const T&, const DeclarationContext*) -> DEnumWrapper<T>;


    class Class;

    class DNamespace : public Declaration, public DeclarationContext {
    public:
        using Declaration::Declaration;

        virtual ~DNamespace() = 0;
        // virtual const std::vector<const DNamespace*> getNamespaces() const noexcept = 0;
        // virtual const std::vector<const DEnum*> getEnums() const noexcept = 0;
        // virtual const std::vector<const DClass*> getClasses() const noexcept = 0;
        // virtual const DNamespace *findChildNamespace(std::string_view name) const noexcept(false) = 0;
        const DNamespace *asNamespace() const noexcept final;
        const DeclarationContext *asDeclContext() const noexcept final;
    private:
    };

    class DParameter : public Declaration, public DTypedDeclaration {
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

        inline virtual const DType *getType() const noexcept final {
            return &type;
        }

    private:
        using param_type = DTypeWrapper<decltype(Descriptor::type)>;
        static const param_type type;
    };

    template<typename Descriptor>
    const typename DParameterWrapper<Descriptor>::param_type DParameterWrapper<Descriptor>::type(Descriptor::type);


    class DMethod : public Declaration {
    public:
        using Declaration::Declaration;
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

        virtual const DType *getReturnType() const noexcept = 0;
        const DMethod *getNextOverload() const noexcept;

        void pushOverload(std::unique_ptr<DMethod> &&next);
    private:
        std::unique_ptr<DMethod> nextOverload = nullptr;
    };

    template <typename Descriptor>
    class DMethodWrapper : public DMethod {
    public:

        DMethodWrapper(const Descriptor& desc, const DeclarationContext* parent)
            : DMethod(parent),
              descriptor(desc) {
        }

        inline virtual std::string_view getName() const noexcept final {
            return descriptor.name;
        }

        virtual void call(const void *object, void *retValAddr, void **args) const final {
            if constexpr (Descriptor::is_const) {
                return descriptor.invoke(object, retValAddr, args);
            } else {
                throw const_corectness_error("non const method called on const object");
            }
        }

        inline virtual void call(void *object, void *retValAddr, void **args) const final {
            descriptor.invoke(object, retValAddr, args);
        }

        inline virtual const DType *getReturnType() const noexcept final {
            return nullptr; //  &returntype;
        }

    private:
        // using parameter_model = detail::range_model<typename Descriptor::parameters, DParameter, DParameterWrapper>;
        // static constexpr parameter_model parameters {};

        // using return_type = DTypeWrapper<Descriptor::return_type>;
        // static const return_type returntype;

        Descriptor descriptor;
    };

    template <typename T>
    DMethodWrapper(const T&, const DeclarationContext*) -> DMethodWrapper<T>;

    template <typename T>
    auto makeUniqueMethod(const T& d, const DeclarationContext*p) {
        return std::make_unique<DMethodWrapper<T>>(d, p);
    }

    template <typename T>
    auto makeMethod(const T& d, const DeclarationContext*p) {
        return new DMethodWrapper<T>(d, p);
    }


    // template <typename Descriptor>
    // const typename DMethodWrapper<Descriptor>::return_type DMethodWrapper<Descriptor>::returntype(Descriptor::return_type);

    class DField : public Declaration, public DTypedDeclaration {
    public:
        using Declaration::Declaration;

        virtual ~DField() = 0;
        virtual void assign_copy(void* o, void* a) const = 0;
        virtual void assign_move(void* o, void* a) const = 0;
    };

    template <typename Descriptor>
    class DFieldWrapper : public DField {
    public:
        inline virtual ~DFieldWrapper() = default;

        DFieldWrapper(const Descriptor &desc, const DeclarationContext *parent)
            :DField(parent),
             descriptor(desc) {}

        inline virtual std::string_view getName() const noexcept final {
            return descriptor.name;
        }

        inline virtual const DType *getType() const noexcept final {
            return nullptr; // &type;
        }

        inline virtual void assign_copy(void* o, void* a) const final {
            descriptor.assign_copy(o, a);
        }

        inline virtual void assign_move(void* o, void* a) const final {
            descriptor.assign_move(o, a);
        }

    private:
        // using type_information = DTypeWrapper<decltype(Descriptor::type)>;
        // static const type_information type;
        Descriptor descriptor;
    };

    template <typename T>
    auto makeField(const T &d, const DeclarationContext* p) {
        return std::make_unique<DFieldWrapper<T>>(d, p);
    }

    // template <typename Descriptor>
    // const typename DFieldWrapper<Descriptor>::type_information DFieldWrapper<Descriptor>::type(Descriptor::type);

    class Class : public TypeDeclaration, public DeclarationContext {
    public:
        using TypeDeclaration::TypeDeclaration;
        virtual ~Class() = 0;
        // virtual const DMethod *findMethod(std::string_view name) const noexcept = 0;
        // virtual const DField *findField(std::string_view name) const noexcept(false) = 0;
        const Class *asClass() const noexcept final;
        const DeclarationContext *asDeclContext() const noexcept final;
    private:
    };

    template <typename MetaClass>
    class ClassWrapper : public Class {
    public:
        using descriptor = MetaClass;

        ClassWrapper(const MetaClass &mc, const DeclarationContext *parent)
            : Class(parent) {

            initMethods();
            initEnums();
            initFields();
        }

        inline ~ClassWrapper() = default;

        inline std::string_view getName() const noexcept final {
            return descriptor::name;
        }

        /*inline const DMethod *findMethod(std::string_view name) const noexcept final {
            auto res = method_map.find(name);
            if (res != method_map.end()) {
                return res->second[0].get();
            }

            return nullptr;
        }

        inline const DClass *findChildClass(std::string_view name) const noexcept(false) final {
            return nullptr; // classes.element_map.at(name);
        }

        inline const DEnum *findChildEnum(std::string_view name) const noexcept(false) final {
            return nullptr; // enums.element_map.at(name);
        }

        inline const DField *findField(std::string_view name) const noexcept(false) final {
            auto res = field_map.find(name);
            if (res != field_map.end()) {
                return res->second.get();
            }

            return nullptr;
        }*/

        inline const Declaration *getDeclaration(std::string_view name) const noexcept final {
            return nullptr; // TDO DUUUUUH
        }

    private:
        // using enum_range = detail::range_model<typename MetaClass::enums, DEnum, DEnumWrapper>;
        // static constexpr enum_range enums = {};

        // template<typename T>
        // using disambiguator = DClassWrapper<T>;

        // using class_range = detail::range_model<typename MetaClass::classes, DClass, disambiguator>;
        // static constexpr class_range classes = {};

        void initMethods() {
            std::unordered_map<std::string_view, std::unique_ptr<DMethod>> all_methods;
            std::apply([&all_methods, this](auto &&...mts) {
                bool added;
                (((added = static_cast<bool>(all_methods[mts.name])), (added ? all_methods[mts.name]->pushOverload(makeUniqueMethod(mts, this)) : static_cast<void>(all_methods[mts.name] = makeUniqueMethod(mts, this)))), ...);
            }, descriptor::methods);
            for (auto& [nm, pv]: all_methods) {
                declarations[nm] = std::move(pv);
            }
        }

        void initFields() {
            std::apply([this](auto &&...fields) {
                ((declarations[fields.name] = makeField(fields, this)), ...);
            }, descriptor::fields);
        }

        void initEnums() {
            using enums_type = typename descriptor::enums;
            enums_type enums;
            std::apply([this](auto &&...enms) {
                ((declarations[enms.name] = makeEnum(enms, this)), ...);
            }, enums);
        }

        std::unordered_map<std::string_view, std::unique_ptr<Declaration>> declarations;
    };

    template <typename T>
    DClassWrapper(const T&, const DeclarationContext*) -> DClassWrapper<T>;


    template<typename MetaNamespace>
    class DNamespaceWrapper;

    template <typename T>
    auto makeNamespace(const T &v, const DeclarationContext *p) {
        return std::make_unique<DNamespaceWrapper<T>>(v, p);
    }

    template <typename T>
    auto makeClass(const T &v, const DeclarationContext *p) {
        return std::make_unique<ClassWrapper<T>>(v, p);
    }

    template <typename T>
    auto makeEnum(const T &v, const DeclarationContext *p) {
        return std::make_unique<DEnumWrapper<T>>(v, p);
    }


    template<typename MetaNamespace>
    class DNamespaceWrapper : public DNamespace {
        using descriptor = MetaNamespace;
    public:

        DNamespaceWrapper(const MetaNamespace& mn, const DeclarationContext *parent)
            : DNamespace(parent) {
            initClasses();
            initNamespaces();
        }

        inline virtual ~DNamespaceWrapper() = default;

        inline virtual std::string_view getName() const noexcept {
            return descriptor::name;
        }

        inline const Declaration *getDeclaration(std::string_view name) const noexcept final {
            auto res = declarations.find(name);
            return res != declarations.end() ? res->second.get() : nullptr;
        }

    protected:

        void initEnums() {
            using enums_type = typename MetaNamespace::enums;
            enums_type enums;
            std::apply([this](auto &&...enms) {
                ((declarations[enms.name] = makeEnum(enms, this)), ...);
            }, enums);
        }

        void initClasses() {
            using classes_tuple = typename MetaNamespace::classes;
            classes_tuple ctup;
            std::apply([this](auto &&...clses) {
                ((declarations[clses.name] = makeClass(clses, this)), ...);
            }, ctup);
        }

        void initNamespaces() {
            using namespaces_tuple = typename MetaNamespace::namespaces;
            namespaces_tuple namespaces;

            std::apply([this](auto &&...nmspcs) {
                ((declarations[nmspcs.name] = makeNamespace(nmspcs, this)), ...);
            }, namespaces);
        }
        std::unordered_map<std::string_view, std::unique_ptr<Declaration>> declarations;
    };

    template <typename T>
    DNamespaceWrapper(const T&, const DeclarationContext*) -> DNamespaceWrapper<T>;

}
