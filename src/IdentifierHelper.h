#pragma once

#pragma warning(push, 0)
#include <clang/AST/PrettyPrinter.h>
#include <llvm/Support/raw_ostream.h>
#pragma warning(pop)

namespace clang {
    class CXXRecordDecl;
    class EnumDecl;
    class FunctionDecl;
    class Decl;
    class CXXMethodDecl;
    class ClassTemplateSpecializationDecl;

    // types we deal with
    class Type;
    class QualType;
    class BuiltinType;
    class PointerType;
    class LValueReferenceType;
    class RValueReferenceType;
    class TypedefType;
    class EnumType;
    class RecordType;
    class ElaboratedType;
    class TemplateSpecializationType;

    class ASTContext;
}
#include <fstream>
#include <string_view>
#include <string>
#include <map>

namespace mc {
std::string replaceIllegalIdentifierChars(std::string_view name);

const constexpr std::string_view g_Type_Prefix = "__metal__Ty__";
const constexpr std::string_view g_Decl_Prefix = "__metal__Decl__";

    /**
     * @brief The IdentifierHelper class is essentially unused right now but it might come in handy so it's keep just a little longer
     */
    class IdentifierHelper {
    public:
        explicit IdentifierHelper(const clang::PrintingPolicy &pPolicy)
            :printingPolicy(pPolicy) {}

        std::string id(const clang::Type *ty, std::string_view prefix = g_Type_Prefix);
        std::string id(const clang::QualType &ty, std::string_view prefix = "");
        std::string id(const clang::BuiltinType *ty, std::string_view prefix = "");
        std::string id(const clang::PointerType *ty, std::string_view prefix = "");
        std::string id(const clang::LValueReferenceType *ty, std::string_view prefix = "");
        std::string id(const clang::RValueReferenceType *ty, std::string_view prefix = "");
        std::string id(const clang::TypedefType *ty, std::string_view prefix = "");
        std::string id(const clang::EnumType *ty, std::string_view prefix = "");
        std::string id(const clang::RecordType *ty, std::string_view prefix = "");
        std::string id(const clang::ElaboratedType *ty, std::string_view prefix = "");
        std::string id(const clang::TemplateSpecializationType *ty, std::string_view prefix = "");

        std::string id(const clang::Decl *d, std::string_view prefix = g_Decl_Prefix);
        std::string id(const clang::CXXMethodDecl *d, std::string_view prefix = "");
        std::string id(const clang::CXXRecordDecl *d, std::string_view prefix = "");
        std::string id(const clang::ClassTemplateSpecializationDecl *d, std::string_view prefix = "");

    private:
        /* a series of maps to avoid recomputing identifiers for the same names */
        std::map<const clang::Type*, std::string> typeIdentifierMap;
        std::map<const clang::Decl*, std::string> declIdentifierMap;

        /* end of maps */
        const clang::PrintingPolicy& printingPolicy;
        // std::string outJson;
    };
}

