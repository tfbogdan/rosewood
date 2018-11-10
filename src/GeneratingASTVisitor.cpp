#include "GeneratingASTVisitor.h"

#include <unordered_map>
#include <string_view>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <experimental/filesystem>

#pragma warning(push, 0)
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

#define putname ownScope.putline("static constexpr std::string_view name = \"{}\";", name)

namespace mc {
    namespace fs = std::experimental::filesystem;


    ReflectionDataGenerator::ReflectionDataGenerator(const clang::ASTContext &astContext)
        :out(mcOutput),
        idman(astContext.getPrintingPolicy()),
        idrepo(),
        context(astContext),
        printingPolicy(astContext.getPrintingPolicy()) {

        global_scope.putline("#pragma once");
        global_scope.putline("#include <string_view>");
        global_scope.putline("#include <tuple>");
        global_scope.putline("");
        global_scope.putline("namespace mc {{");
    }

    ReflectionDataGenerator::~ReflectionDataGenerator() {
        global_scope.putline("}} //::mc");
        idrepo.save(mcJsonOutput.getValue());
        out.flush();
    }

    void ReflectionDataGenerator::Generate() {
        descriptor_scope module_scope = descriptor_scope(global_scope.spawn(), mcModuleName.getValue(), mcModuleName.getValue());

        for(const auto decl: context.getTranslationUnitDecl()->decls()) {
            // first cull out everything that isn't defined withing the `main` file
            const bool inMainFile(context.getSourceManager().isInMainFile(decl->getLocation()));
            if (inMainFile) {
                exportDeclaration(decl, module_scope);
            }
        }
    }

    void ReflectionDataGenerator::exportDeclaration(const clang::Decl *Decl, descriptor_scope &where) {
        // so this is a sort of a type dispatcher
        switch(auto declKind = Decl->getKind()) {
        case clang::Decl::Kind::Namespace:
            exportNamespace(static_cast<const clang::NamespaceDecl*>(Decl), where);
            break;
        case clang::Decl::Kind::Enum:
            exportEnum(static_cast<const clang::EnumDecl*>(Decl), where);
            break;
        case clang::Decl::Kind::CXXRecord: {
            auto record = static_cast<const clang::CXXRecordDecl*>(Decl);
            if (record->isThisDeclarationADefinition()) {
                exportCxxRecord(static_cast<const clang::CXXRecordDecl*>(Decl), where);
            }
        } break;
        case clang::Decl::Kind::CXXConstructor: {
            exportCxxConstructor(static_cast<const clang::CXXConstructorDecl*>(Decl), where);
        } break;
        case clang::Decl::Kind::CXXMethod: {
            exportCxxMethod(static_cast<const clang::CXXMethodDecl*>(Decl), where);
        }break;
        default:
            break;
        }
    }


    void ReflectionDataGenerator::exportCxxMethod(const clang::CXXMethodDecl *Method, descriptor_scope &where) {
        if (!Method->isOverloadedOperator()) {
            auto qualName = Method->getQualifiedNameAsString();
            auto name = Method->getNameAsString();
            auto ownScope = where.spawn(name, qualName);
            putname;
            ownScope.putline("using return_type = {};", Method->getReturnType().getAsString(printingPolicy));
            std::vector<clang::ParmVarDecl*> parameters(Method->parameters());
            for (const auto param: parameters) {
                auto parmName = param->getNameAsString();
                auto paramScope = ownScope.spawn(parmName, parmName);
                paramScope.putline("static constexpr std::string_view name = \"{}\";", parmName);
                ownScope.putline("using type = {};", param->getType().getAsString(printingPolicy));
            }

            const std::size_t numParams = parameters.size();
            ownScope.putline("using parameters = std::tuple<");
            auto initScope = ownScope.inner.spawn();
            for(std::size_t idx(0); idx < numParams; ++idx) {
                const auto& parameter = parameters[idx];
                auto parmName = parameter->getNameAsString();
                initScope.indent();
                initScope.rawput(parmName);
                if (idx < (numParams - 1)) {
                    initScope.rawput(",");
                }
                initScope.rawput("\n");
            }

            ownScope.putline(">;");
        }
    }

    void ReflectionDataGenerator::exportCxxConstructor(const clang::CXXConstructorDecl *, descriptor_scope &) {
        /// whelp... all ctors have the same names so we need to come up with an overloading mechanism
        /// but that can get a bit hairy for let's skip this till we're done with the simple stuff
    }

    template <typename declRangeT>
    void fold_range(std::string_view withName, scope where, const declRangeT &range) {
        const std::size_t numElements = std::size(range);
        if (numElements == 0) {
            where.putline("using {} = std::tuple<>;", withName);
            return;
        }

        where.putline("using {} = std::tuple<", withName);
        auto initScope = where.spawn();

        for(std::size_t idx(0); idx < numElements; ++idx) {
            const auto& item = range[idx];
            auto itemName = item->getNameAsString();
            initScope.indent();
            initScope.rawput(itemName);
            if (idx < (numElements - 1)) {
                initScope.rawput(",");
            }
            initScope.rawput("\n");
        }
        where.putline(">;");
    }

    void ReflectionDataGenerator::exportCxxRecord(const clang::CXXRecordDecl *Record, descriptor_scope &where) {
        auto qualName = Record->getQualifiedNameAsString();
        auto name = Record->getNameAsString();
        auto ownScope = where.spawn(name, qualName);

        std::vector<const clang::CXXMethodDecl*> methods;

        for(const auto Method: Record->methods()) {
            const bool isCtor = Method->getKind() == clang::Decl::Kind::CXXConstructor;
            const bool overloadedOperator = Method->isOverloadedOperator();
            const bool isDtor = Method->getKind() == clang::Decl::Kind::CXXDestructor;

            // this captures constructors too so skip those
            if (!overloadedOperator && !isCtor && !isDtor) {
                auto methodQualName = Method->getQualifiedNameAsString();
                auto methodName = Method->getNameAsString();
                auto methodScope = ownScope.spawn(methodName, methodQualName);
                methodScope.putline("static constexpr std::string_view name = \"{}\";", methodName);

                methodScope.putline("using return_type = {};", Method->getReturnType().getAsString(printingPolicy));

                std::vector<clang::ParmVarDecl*> parameters(Method->parameters());
                for (const auto param: parameters) {
                    auto parmName = param->getNameAsString();
                    auto paramScope = methodScope.spawn(parmName, parmName);
                    paramScope.putline("static constexpr std::string_view name = \"{}\";", parmName);
                    methodScope.putline("using type = {};", param->getType().getAsString(printingPolicy));
                }
                fold_range("parameters", methodScope.inner, parameters);
                methods.push_back(Method);
            }
        }

        fold_range("methods", ownScope.inner, methods);
    }

    void ReflectionDataGenerator::exportEnum(const clang::EnumDecl *Enum, descriptor_scope &where) {
        auto qualName = Enum->getQualifiedNameAsString();
        auto name = Enum->getNameAsString();

        auto ownScope = where.spawn(name, qualName);
        ownScope.putline("static constexpr std::string_view name = \"{}\";", name);
        std::vector<clang::EnumConstantDecl*> enumerators;
        for(const auto enumerator: Enum->enumerators()) {
            auto enName = enumerator->getNameAsString();
            auto enScope = ownScope.spawn(enName, enumerator->getQualifiedNameAsString());
            enScope.putline("static constexpr std::string_view name = \"{}\";", enName);
            enScope.putline("static constexpr int64_t value = {};", enumerator->getInitVal().getExtValue());
            enumerators.push_back(enumerator);
        }

        fold_range("enumerators", ownScope.inner, enumerators);
    }

    void ReflectionDataGenerator::exportNamespace(const clang::NamespaceDecl *Namespace, descriptor_scope &where) {
        auto qualName = Namespace->getQualifiedNameAsString();
        auto name = Namespace->getNameAsString();
        auto ownScope = where.spawn(name, qualName);
        ownScope.putline("static constexpr std::string_view name = \"{}\";", name);
        for(const auto decl: Namespace->decls()) {
            exportDeclaration(decl, ownScope);
        }
    }

    void ReflectionDataGenerator::genTypeDescriptor(const clang::Type *ty) {
        return;
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
