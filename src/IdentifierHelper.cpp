#include "IdentifierHelper.h"

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <sstream>

#pragma warning(push, 0)
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Path.h>
#include <clang/AST/Type.h>
#include <clang/AST/AST.h>
#include <clang/AST/ASTContext.h>
#pragma warning(pop)

#include <map>
// #include <nlohmann/json.hpp>

namespace mc {

        const std::map<std::string, std::string> specialCharacterMap{
        { "*", "_ptr_" },
        { "&", "_lvRef_" },
        { "&&","_rvRef_" },
        { "<", "_lt_" },
        { ">", "_gt_" },
        { "::", "_sr_" },
        { " ", "_" },
        { ",", "_comma_" },
        { ".", "_dot_"},
        { "(", "_Pa_"},
        { ")", "_aP_"}
    };

    std::string replaceIllegalIdentifierChars(std::string_view name) {
        std::string res(name);
        for (const auto &illegalSeq : specialCharacterMap) {
            auto index = res.find(illegalSeq.first, 0);
            while (index != std::string::npos) {
                res.erase(index, illegalSeq.first.length());
                res.insert(index, illegalSeq.second);
                index = res.find(illegalSeq.first, 0);
            }
        }
        return res;
    }

    constexpr std::string_view sif(bool value, std::string_view iftrue, std::string_view otherwise="") {
        return value ? iftrue : otherwise;
    }

    const constexpr std::string_view no_prefix;

    std::string IdentifierHelper::id(const clang::Type *ty, std::string_view prefix) {
        auto res(typeIdentifierMap.find(ty));
        if (res != typeIdentifierMap.end()) {
            return fmt::format("{}{}", prefix, res->second);
        }

        std::string unprefixedRes;

        switch (ty->getTypeClass()) {
        case clang::Type::TypeClass::Builtin:
            unprefixedRes = id(static_cast<const clang::BuiltinType*>(ty), "");
            break;
        case clang::Type::TypeClass::Pointer:
            unprefixedRes = id(static_cast<const clang::PointerType*>(ty), "");
            break;
        case clang::Type::TypeClass::LValueReference:
            unprefixedRes = id(static_cast<const clang::LValueReferenceType*>(ty), "");
            break;
        case clang::Type::TypeClass::RValueReference:
            unprefixedRes = id(static_cast<const clang::RValueReferenceType*>(ty), "");
            break;
        case clang::Type::TypeClass::Typedef:
            unprefixedRes = id(static_cast<const clang::TypedefType*>(ty), "");
            break;
        case clang::Type::TypeClass::Enum:
            unprefixedRes = id(static_cast<const clang::EnumType*>(ty), "");
            break;
        case clang::Type::TypeClass::Record:
            unprefixedRes = id(static_cast<const clang::RecordType*>(ty), "");
            break;
        case clang::Type::TypeClass::Elaborated:
            unprefixedRes = id(static_cast<const clang::ElaboratedType*>(ty), "");
            break;
        case clang::Type::TypeClass::TemplateSpecialization:
            unprefixedRes = id(static_cast<const clang::TemplateSpecializationType*>(ty), "");
            break;
        default:
            assert(false);  // unhandled typeclass
            break;
        }
        typeIdentifierMap[ty] = unprefixedRes;
        return std::string(prefix) + unprefixedRes;
    }

    std::string IdentifierHelper::id(const clang::QualType &ty, std::string_view prefix) {
        return fmt::format("{}{}{}{}{}",
                           prefix,
                           sif(ty.isConstQualified(), "cq_"),
                           sif(ty.isVolatileQualified(), "vq_"),
                           sif(ty.isRestrictQualified(), "rq_"),
                           id(ty.getTypePtr(), no_prefix));
    }

    std::string IdentifierHelper::id(const clang::BuiltinType *ty, std::string_view prefix) {
        return fmt::format("{}{}", prefix, replaceIllegalIdentifierChars(ty->getName(printingPolicy).str()));
    }

    std::string IdentifierHelper::id(const clang::PointerType *ty, std::string_view prefix) {
        return fmt::format("{}{}{}", prefix, "_ptr_", id(ty->getPointeeType()));
    }

    std::string IdentifierHelper::id(const clang::LValueReferenceType *ty, std::string_view prefix) {
        return fmt::format("{}{}{}", prefix, "_lvRef_", id(ty->getPointeeType()));
    }

    std::string IdentifierHelper::id(const clang::RValueReferenceType *ty, std::string_view prefix) {
        return fmt::format("{}{}{}", prefix, "_rvRef_", id(ty->getPointeeType()));
    }

     std::string IdentifierHelper::id(const clang::TypedefType *ty, std::string_view prefix) {
        const clang::TypedefNameDecl *typedefDecl = ty->getDecl();
        return fmt::format("{}{}{}", prefix, "_td_", replaceIllegalIdentifierChars(typedefDecl->getQualifiedNameAsString()));
    }

    std::string IdentifierHelper::id(const clang::EnumType *ty, std::string_view prefix) {
        const clang::EnumDecl *enumDecl = ty->getDecl();
        return fmt::format("{}{}{}", prefix, "_en_", replaceIllegalIdentifierChars(enumDecl->getQualifiedNameAsString()));
    }

    std::string IdentifierHelper::id(const clang::RecordType *ty, std::string_view prefix) {
        const clang::RecordDecl *recDecl = ty->getDecl();
        return fmt::format("{}{}{}", prefix, "_rc_", replaceIllegalIdentifierChars(recDecl->getQualifiedNameAsString()));
    }

    std::string IdentifierHelper::id(const clang::ElaboratedType *ty, std::string_view prefix) {
        return fmt::format("{}{}{}", prefix, "_et_", id(ty->getNamedType(), no_prefix));
    }

    std::string IdentifierHelper::id(const clang::TemplateSpecializationType *ty, std::string_view prefix) {
        // llvm::raw_string_ostream os(res);
        std::ostringstream os;

        os << "_ts_";
        if (ty->isTypeAlias()) {
            os << fmt::format("{}{}", "_ts_", id(ty->getAliasedType()));
        }
        else {
            auto rec(static_cast<const clang::ClassTemplateSpecializationDecl*>(ty->getAsCXXRecordDecl()));
            llvm::SmallVector<char, 1024> rres;
            llvm::raw_svector_ostream oos(rres);
            os << replaceIllegalIdentifierChars(rec->getQualifiedNameAsString());
            bool firstArg(true);
            oos << "<";
            for (const auto &arg : rec->getTemplateArgs().asArray()) {
                if (firstArg) { firstArg = false; }
                else {
                    oos << ", ";
                }
                arg.print(printingPolicy, oos);
            }
            oos << ">";
            os << replaceIllegalIdentifierChars(oos.str().str());
        }

        return fmt::format("{}{}", prefix, os.str());
    }


    std::string IdentifierHelper::id(const clang::Decl *d, std::string_view prefix) {
        auto res(declIdentifierMap.find(d));
        if (res != declIdentifierMap.end()) {
            return std::string(prefix) + res->second;
        }

        std::string unprefixedRes;
        switch (d->getKind()) {
        case clang::Decl::Kind::CXXMethod: {
            auto method = static_cast<const clang::CXXMethodDecl*>(d);
            unprefixedRes = id(method);
        } break;
        case clang::Decl::Kind::ClassTemplateSpecialization: {
            unprefixedRes = id(static_cast<const clang::ClassTemplateSpecializationDecl*>(d));
        } break;
        case clang::Decl::Kind::CXXRecord: {
            auto record = static_cast<const clang::CXXRecordDecl*>(d);
            unprefixedRes = id(record);
        } break;
        default:
            break;
        }

        declIdentifierMap[d] = unprefixedRes;
        return fmt::format("{}{}", prefix, unprefixedRes);
    }

    std::string IdentifierHelper::id(const clang::CXXMethodDecl *d, std::string_view prefix) {
        return fmt::format("{}{}{}", prefix, "_mth_", replaceIllegalIdentifierChars(d->getQualifiedNameAsString()));
    }

    std::string IdentifierHelper::id(const clang::CXXRecordDecl *d, std::string_view prefix) {
        return fmt::format("{}{}{}", prefix, "_record_", replaceIllegalIdentifierChars(d->getQualifiedNameAsString()));
    }

    std::string IdentifierHelper::id(const clang::ClassTemplateSpecializationDecl *d, std::string_view prefix) {
        std::ostringstream os;
        os << "_ts_" << replaceIllegalIdentifierChars(d->getQualifiedNameAsString());

        llvm::SmallString<512> tStr;
        llvm::raw_svector_ostream tos(tStr);

        tos << "<";
        bool firstArg(true);
        for (const auto &arg : d->getTemplateArgs().asArray()) {

            if (firstArg) { firstArg = false; }
            else {
                tos << ", ";
            }
            arg.print(printingPolicy, tos);
        }
        tos << ">";
        os << replaceIllegalIdentifierChars(tos.str().str());
        return fmt::format("{}{}", prefix, os.str());
    }
}
