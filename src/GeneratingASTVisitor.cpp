#include "GeneratingASTVisitor.h"

#include <unordered_map>
#include <string_view>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <experimental/filesystem>

#pragma warning(push, 0)
#include <llvm/Support/CommandLine.h>
#pragma warning(pop)

extern llvm::cl::opt<std::string> mcOutput;
extern llvm::cl::opt<std::string> mcJsonOutput;

static const std::unordered_map<clang::BuiltinType::Kind, std::string_view> builtinTypes {
    { clang::BuiltinType::Kind::Void, "BK_Void"},
    { clang::BuiltinType::Kind::Char_S, "BK_Char"},
    { clang::BuiltinType::Kind::Char_U, "BK_Char"},
    { clang::BuiltinType::Kind::WChar_S, "BK_WChar"},
    { clang::BuiltinType::Kind::WChar_U, "BK_UWChar"},
    { clang::BuiltinType::Kind::Short, "BK_Short"},
    { clang::BuiltinType::Kind::UShort, "BK_UShort"},
    { clang::BuiltinType::Kind::Int, "BK_Int"},
    { clang::BuiltinType::Kind::UInt, "BK_UInt"},
    { clang::BuiltinType::Kind::Long, "BK_Long"},
    { clang::BuiltinType::Kind::ULong, "BK_ULong"},
    { clang::BuiltinType::Kind::LongLong, "BK_LongLong"},
    { clang::BuiltinType::Kind::ULongLong, "BK_ULongLong"},
    { clang::BuiltinType::Kind::Float, "BK_Float"},
    { clang::BuiltinType::Kind::Double, "BK_Double"},
    { clang::BuiltinType::Kind::Bool, "BK_Bool"}
};

namespace mc {
    namespace fs = std::experimental::filesystem;

    GeneratingASTVisitor::GeneratingASTVisitor(const clang::PrintingPolicy &pPolicy)
        :out(mcOutput),
        idman(pPolicy),
        idrepo(),
        printingPolicy(pPolicy) {

        out << "#include <mc/MetaType.h>\n";
    }

    GeneratingASTVisitor::~GeneratingASTVisitor() {
        idrepo.save(mcJsonOutput.getValue());
    }

    
    bool GeneratingASTVisitor::VisitCXXRecordDecl(const clang::CXXRecordDecl *record) {
        const bool inMainFile(record->getASTContext().getSourceManager().isInMainFile(record->getLocation()));

        if (inMainFile && record->isThisDeclarationADefinition()) {
            for(const auto method: record->methods()) {
                if (method->getAccess() == clang::AccessSpecifier::AS_public) {
                    genTypeDescriptor(method->getReturnType().getTypePtr());
                    for(const auto arg: method->parameters()) {
                        genTypeDescriptor(arg->getType().getTypePtr());
                    }
                }
            }
        }

        return true;
    }

    bool GeneratingASTVisitor::VisitEnumDecl(const clang::EnumDecl *Enum) {
        if (exportDeclaration(Enum)) {
            exportedEnums.push_back(Enum);
        }
        return true;
    }

    bool GeneratingASTVisitor::VisitCXXMethodDecl(const clang::CXXMethodDecl *) {
        // only deals with filtering methods from functions
        // methods are already handled in the context of their classes
        return true;
    }

    bool GeneratingASTVisitor::VisitFunctionDecl(const clang::FunctionDecl *Function) {
        if (exportDeclaration(Function)) {
            exportedFunctions.push_back(Function);
        }
        return true;
    }

    bool GeneratingASTVisitor::exportDeclaration(const clang::Decl *Decl) {
        bool isInMainFile(Decl->getASTContext().getSourceManager().isInMainFile(Decl->getLocation()));
        // for now, everything that is in a file parsed explicitly is exported.
        // this should be extended to skip stuff defined in an internal namespace
        return isInMainFile;
    }

    void GeneratingASTVisitor::genTypeDescriptor(const clang::Type *ty) {
        auto identifier = idman.id(ty);

        if (idrepo.isDefined(identifier)) return;

        switch (ty->getTypeClass()) {
        case clang::Type::TypeClass::Builtin: {
            // auto btIn = static_cast<const clang::BuiltinType*>(ty);
            // const auto mcBuiltInName = builtinTypes.at(btIn->getKind());
            // fmt::print(out, "const metal::BuiltinType {}(metal::BuiltinType::BuiltinKind::{}, \"{}\");",
            //            identifier, mcBuiltInName, btIn->getCanonicalTypeInternal().getAsString(printingPolicy));
            fmt::print(out, "const extern metal::BuiltinType {};\n",
                       identifier);
            idrepo.expectExternalIdentifier(identifier);
        } break;

        case clang::Type::TypeClass::Pointer: {
            auto pTy = static_cast<const clang::PointerType *>(ty);
            auto pointee = pTy->getPointeeType();
            auto pteeIdent(idman.id(pointee.getTypePtr()));
            if (!idrepo.isDefined(pteeIdent))
                genTypeDescriptor(pointee.getTypePtr());
            fmt::print(out, "const metal::PointerType {} (\n\tmetal::QualType(\n\t\t{}, \n\t\t{}, \n\t\t{}, \n\t\t&{}\n\t), \n\t\"{}\"\n);\n", identifier, pointee.isConstQualified(), pointee.isVolatileQualified(), pointee.isRestrictQualified(), pteeIdent, pointee.getAsString(printingPolicy));
        } break;

        case clang::Type::TypeClass::LValueReference: {
            auto pTy = static_cast<const clang::LValueReferenceType *>(ty);
            auto pointee = pTy->getPointeeType();
            auto pteeIdent(idman.id(pointee.getTypePtr()));
            if (!idrepo.isDefined(pteeIdent))
                genTypeDescriptor(pointee.getTypePtr());
            fmt::print(out, "const metal::LReferenceType {} (\n\tmetal::QualType(\n\t\t{}, \n\t\t{}, \n\t\t{}, \n\t\t&{}\n\t), \n\t\"{}\"\n);\n", identifier, pointee.isConstQualified(), pointee.isVolatileQualified(), pointee.isRestrictQualified(), pteeIdent, pointee.getAsString(printingPolicy));
        } break;

        case clang::Type::TypeClass::RValueReference: {
            auto pTy = static_cast<const clang::RValueReferenceType *>(ty);
            auto pointee = pTy->getPointeeType();
            auto pteeIdent(idman.id(pointee.getTypePtr()));
            if (!idrepo.isDefined(pteeIdent))
                genTypeDescriptor(pointee.getTypePtr());
            fmt::print(out, "const metal::RReferenceType {} (\n\tmetal::QualType(\n\t\t{}, \n\t\t{}, \n\t\t{}, \n\t\t&{}\n\t), \n\t\"{}\"\n);\n", identifier, pointee.isConstQualified(), pointee.isVolatileQualified(), pointee.isRestrictQualified(), pteeIdent, pointee.getAsString(printingPolicy));
        } break;

        case clang::Type::TypeClass::Typedef: {
            auto tty = static_cast<const clang::TypedefType*>(ty);
            auto tdecl = tty->getDecl();
            auto ttIdent(idman.id(tdecl->getUnderlyingType().getTypePtr()));
            if (!idrepo.isDefined(ttIdent))
                genTypeDescriptor(tdecl->getUnderlyingType().getTypePtr());
            fmt::print(out, "const metal::TypedefType {} (\n\tmetal::QualType(\n\t\t{}, \n\t\t{}, \n\t\t{}, \n\t\t&{}\n\t), \n\t\"{}\"\n);\n", identifier, tdecl->getUnderlyingType().isConstQualified(), tdecl->getUnderlyingType().isVolatileQualified(), tdecl->getUnderlyingType().isRestrictQualified(), ttIdent, tdecl->getQualifiedNameAsString());
        } break;

        case clang::Type::TypeClass::Elaborated: {
            auto ety = static_cast<const clang::ElaboratedType*>(ty);
            auto ttIdent(idman.id(ety->getNamedType().getTypePtr()));
            if (!idrepo.isDefined(ttIdent))
                genTypeDescriptor(ety->getNamedType().getTypePtr());

            fmt::print(out, "const metal::TypedefType {} (\n\tmetal::QualType(\n\t\t{}, \n\t\t{}, \n\t\t{}, \n\t\t&{}\n\t), \n\t\"{}\"\n);\n", identifier, ety->getNamedType().isConstQualified(), ety->getNamedType().isVolatileQualified(), ety->getNamedType().isRestrictQualified(), ttIdent, ety->getNamedType().getAsString(printingPolicy));
        } break;

        case clang::Type::TypeClass::TemplateSpecialization: {
            auto tty = static_cast<const clang::TemplateSpecializationType*>(ty);
            auto decl = tty->getAsCXXRecordDecl();
            const bool isInMainFile(decl->getASTContext().getSourceManager().isInMainFile(decl->getLocation()));

            if (isInMainFile) {
                llvm::SmallVector<char, 255> ttt;
                llvm::raw_svector_ostream ot(ttt);

                auto spec = static_cast<const clang::ClassTemplateSpecializationDecl*>(decl);
                spec->printQualifiedName(ot);
                ot << "<";
                bool firstArg(true);
                for (const auto &arg : spec->getTemplateArgs().asArray()) {
                    if (firstArg) {
                        firstArg = false;
                    } else {
                        ot << ", ";
                    }
                    arg.print(printingPolicy, ot);
                }
                ot << ">";
                fmt::print(out, "const metal::RecordType {} ({}, \"{}\");\n",
                           identifier, idman.id(static_cast<const clang::Decl *>(tty->getAsCXXRecordDecl())), tty->getAsCXXRecordDecl()->getQualifiedNameAsString());
            } else {
                idrepo.expectExternalIdentifier(identifier);
                fmt::print(out, "extern const metal::RecordType {};\n", identifier);
            }

        } break;

        case clang::Type::TypeClass::Record: {
            auto tty = static_cast<const clang::RecordType*>(ty);

            const bool isInMainFile(tty->getDecl()->getASTContext().getSourceManager().isInMainFile(tty->getDecl()->getLocation()));
            if (isInMainFile) {
                fmt::print(out, "const metal::RecordType {} ({}, \"{}\");\n",
                           identifier, idman.id(static_cast<const clang::Decl *>(tty->getAsCXXRecordDecl())), tty->getDecl()->getQualifiedNameAsString());
            } else {
                idrepo.expectExternalIdentifier(identifier);
                fmt::print(out, "extern const metal::RecordType {};\n", identifier);
            }

        } break;
        default:
            fmt::print(out, "// Type kind {} is not handled\n", ty->getCanonicalTypeInternal().getAsString(printingPolicy));
            break;
        }
        idrepo.defineIdentifier(identifier);

    }

}
