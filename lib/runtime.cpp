#include <rosewood/runtime.hpp>


namespace rosewood {
    DType::~DType() = default;
    DTypedDeclaration::~DTypedDeclaration() = default;
    DMetaDecl::~DMetaDecl() = default;
    DeclarationContext::~DeclarationContext() = default;
    const DNamespace *DMetaDecl::asNamespace() const noexcept {
        return nullptr;
    }

    const DClass *DMetaDecl::asClass() const noexcept {
        return nullptr;
    }

    const DEnum *DMetaDecl::asEnum() const noexcept {
        return nullptr;
    }

    const DEnumerator *DMetaDecl::asEnumerator() const noexcept {
        return nullptr;
    }

    const DOverloadSet *DMetaDecl::asOverloadSet() const noexcept {
        return nullptr;
    }

    const DMethod *DMetaDecl::asMethod() const noexcept {
        return nullptr;
    }

    const DField *DMetaDecl::asField() const noexcept {
        return nullptr;
    }

    const DeclarationContext *DMetaDecl::asDeclContext() const noexcept {
        return nullptr;
    }

    DEnum::~DEnum() = default;
    const DEnum *DEnum::asEnum() const noexcept {
        return this;
    }

    DField::~DField() = default;
    DEnumerator::~DEnumerator() = default;
    DNamespace::~DNamespace() = default;
    DParameter::~DParameter() = default;
    DOverloadSet::~DOverloadSet() = default;
    DMethod::~DMethod() = default;
    DClass::~DClass() = default;

    const DClass *DClass::asClass() const noexcept {
        return this;
    }
    const DeclarationContext *DClass::asDeclContext() const noexcept {
        return this;
    }

    const DNamespace *DNamespace::asNamespace() const noexcept {
        return this;
    }
    const DeclarationContext *DNamespace::asDeclContext() const noexcept {
        return this;
    }


}
