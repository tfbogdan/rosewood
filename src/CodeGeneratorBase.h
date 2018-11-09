#pragma once

#pragma warning(push, 0)
#include <clang/AST/PrettyPrinter.h>
#include <llvm/Support/raw_ostream.h>
#pragma warning(pop)

#include "SourceFile.h"

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

const constexpr std::string_view g_Type_Prefix = "__metal__Ty__";
const constexpr std::string_view g_Decl_Prefix = "__metal__Decl__";

    class IdentifierHelper {
    public:
        explicit IdentifierHelper(const clang::PrintingPolicy &pPolicy, const std::string& identJson)
            :printingPolicy(pPolicy),
            outJson(identJson) {}

        ~IdentifierHelper();

        bool isDefined(const std::string &identifier);
        void defineIdentifier(const std::string &identifier);
        void expectExternalIdentifier(const std::string &identifier);

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
        std::set<std::string> definedIdents;
        std::set<std::string> externalIdents;

        /* end of maps */
        const clang::PrintingPolicy& printingPolicy;
        std::string outJson;
    };
}

