#include <rosewood/runtime.hpp>


namespace rosewood {
    DType::~DType() = default;
    DTypedDeclaration::~DTypedDeclaration() = default;
    Declaration::~Declaration() = default;
    DeclarationContext::~DeclarationContext() = default;
    TypeDeclaration::~TypeDeclaration() = default;

    Declaration::Declaration(const DeclarationContext *Parent)
        :parentP(Parent) {}

    const DeclarationContext *Declaration::parent() const noexcept {
        return parentP;
    }

    const DNamespace *Declaration::asNamespace() const noexcept {
        return nullptr;
    }

    const Class *Declaration::asClass() const noexcept {
        return nullptr;
    }

    const DEnum *Declaration::asEnum() const noexcept {
        return nullptr;
    }

    const DEnumerator *Declaration::asEnumerator() const noexcept {
        return nullptr;
    }

    const DMethod *Declaration::asMethod() const noexcept {
        return nullptr;
    }

    const DField *Declaration::asField() const noexcept {
        return nullptr;
    }

    const DeclarationContext *Declaration::asDeclContext() const noexcept {
        return nullptr;
    }
    const TypeDeclaration *Declaration::asTypeDeclaration() const noexcept {
        return nullptr;
    }

    DEnum::~DEnum() = default;
    const DEnum *DEnum::asEnum() const noexcept {
        return this;
    }

    const DMethod *DMethod::getNextOverload() const noexcept {
        return nextOverload.get();
    }

    void DMethod::pushOverload(std::unique_ptr<DMethod> &&next) {
        if (nextOverload) {
            next->pushOverload(std::move(nextOverload));
        }
        nextOverload = std::move(next);
    }


    DField::~DField() = default;
    DEnumerator::~DEnumerator() = default;
    DNamespace::~DNamespace() = default;
    DParameter::~DParameter() = default;
    DMethod::~DMethod() = default;
    Class::~Class() = default;

    const Class *Class::asClass() const noexcept {
        return this;
    }
    const DeclarationContext *Class::asDeclContext() const noexcept {
        return this;
    }

    const DNamespace *DNamespace::asNamespace() const noexcept {
        return this;
    }
    const DeclarationContext *DNamespace::asDeclContext() const noexcept {
        return this;
    }

    const TypeDeclaration *TypeDeclaration::asTypeDeclaration() const noexcept {
        return this;
    }


}
