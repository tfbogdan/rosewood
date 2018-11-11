#include "ReflectionDataGenerator.h"

#include <unordered_map>
#include <string_view>
#include <map>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <experimental/filesystem>

#include <iostream>

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
        global_scope.putline("#include <mc/mc.hpp>");
        auto mainFile = astContext.getSourceManager().getMainFileID();
        auto mainFileLoc = astContext.getSourceManager().getComposedLoc(mainFile, 0);
        auto mainFilePath = astContext.getSourceManager().getFilename(mainFileLoc);

        global_scope.putline("#include \"{}\"", mainFilePath);
        global_scope.putline("");
        global_scope.putline("namespace mc {{");
    }

    ReflectionDataGenerator::~ReflectionDataGenerator() {
        global_scope.putline("}}");
        idrepo.save(mcJsonOutput.getValue());
        out.flush();
    }

    void ReflectionDataGenerator::Generate() {
        printingPolicy = context.getPrintingPolicy();
        descriptor_scope module_scope = descriptor_scope(global_scope.spawn(), mcModuleName.getValue(), mcModuleName.getValue(), "mc::Module");

        for(const auto decl: context.getTranslationUnitDecl()->decls()) {
            // first cull out everything that isn't defined within the `main` file
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
            auto ownScope = where.spawn(name, qualName, "mc::Method");
            putname;
            ownScope.putline("using return_type = {};", Method->getReturnType().getAsString(printingPolicy));
            std::vector<clang::ParmVarDecl*> parameters(Method->parameters());
            for (const auto param: parameters) {
                auto parmName = param->getNameAsString();
                auto paramScope = ownScope.spawn(parmName, parmName, "mc::Parameter");
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
    void wrap_range_in_tuple(std::string_view withName, scope where, const declRangeT &range) {
        const std::size_t numElements = std::size(range);
        if (numElements == 0) {
            where.putline("using {} = std::tuple<>;", withName);
            return;
        }

        where.putline("using {} = std::tuple<", withName);
        auto initScope = where.spawn();

        for(std::size_t idx(0); idx < numElements; ++idx) {
            const auto& item = range[idx];
            initScope.indent();
            if constexpr (std::is_same<std::string, typename declRangeT::value_type>::value) {
                initScope.rawput(item);
            } else {
                auto itemName = item->getNameAsString();
                initScope.rawput("meta_{}", itemName);
            }
            if (idx < (numElements - 1)) {
                initScope.rawput(",");
            }
            initScope.rawput("\n");
        }
        where.putline(">;");
    }

    void genMethodDispatcher(descriptor_scope &descScope, const clang::CXXMethodDecl* Method, const clang::CXXRecordDecl *Record) {
        // this should be more elaborate and should consider all method attributes ( is const for example )
        descScope.putline("static constexpr auto call = [] (");
        scope argList = descScope.inner.spawn().spawn();
        const std::size_t numArgs = Method->param_size();
        const bool isConstructor = Method->getKind() == clang::Decl::Kind::CXXConstructor;
        const bool isImplicit = isConstructor ? static_cast<const clang::CXXConstructorDecl*>(Method)->isImplicit() : false;

        if (isConstructor) {
            argList.putline("void *addr{}", numArgs > 0 ? ",": "");
        } else {
            argList.putline("{}{}& obj{}", Method->isConst() ? "const ": "", Record->getQualifiedNameAsString(), numArgs > 0 ? ",": "");
        }

        std::size_t index(0);
        for (const auto arg: Method->parameters()) {
            argList.putline("{} {}{}", arg->getType().getAsString(Method->getASTContext().getPrintingPolicy()),  isImplicit ? fmt::format("implicit_arg_{}", index) : arg->getNameAsString(), index < (numArgs - 1) ? ",": "");
            ++index;
        }
        descScope.putline(") {{");
        if (isConstructor) {
            argList.putline("new (addr) {} {}", Record->getQualifiedNameAsString(), numArgs > 0 ? "(": ";");
        } else {
            argList.putline("return obj.{} (", Method->getNameAsString());
        }
        auto callArgList = argList.spawn().spawn();
        index = 0;
        for (const auto arg: Method->parameters()) {
            callArgList.putline("{} {}", isImplicit ? fmt::format("implicit_arg_{}", index) : arg->getNameAsString(), index < (numArgs - 1) ? ",": "");
            ++index;
        }
        descScope.putline("{}}};", isConstructor ? (numArgs > 0 ? ");" : "") : ");");

        // const ::{0}, args...) { return ::{1}};", Record->getQualifiedNameAsString(), Method->getQualifiedNameAsString());
    }

    void ReflectionDataGenerator::exportCxxRecord(const clang::CXXRecordDecl *Record, descriptor_scope &where) {
        auto qualName = Record->getQualifiedNameAsString();
        auto name = Record->getNameAsString();
        auto ownScope = where.spawn(name, qualName, "mc::Class");
        ownScope.putline("using type = {};", Record->getQualifiedNameAsString());
        std::vector<std::string> method_descriptors;
        // in order to do overloading we need to do three passes over all methods
        std::map<llvm::StringRef, std::vector<const clang::CXXMethodDecl*>> method_groups;
        std::map<std::string, std::vector<const clang::CXXMethodDecl*>> overloaded_operators;
        // std::map<llvm::StringRef, std::vector<const clang::FunctionDecl*>> function_groups;
        // std::map<std::string, std::vector<const clang::FunctionDecl*>> static_operator_groups;
        std::vector<const clang::CXXConstructorDecl*> constructors;
        std::vector<const clang::CXXConversionDecl*> conversions;
        std::vector<const clang::FieldDecl*> fields;
        std::vector<const clang::Decl*> decls;
        const clang::CXXDestructorDecl *destructor;

        for(const auto decl: Record->decls()) {
            // first trim out non public members
            if(decl->getAccess() != clang::AccessSpecifier::AS_public) continue;
            switch (decl->getKind()) {
            case clang::Decl::Kind::CXXMethod: {
                auto method = static_cast<const clang::CXXMethodDecl*>(decl);
                // TDO: the other half
                if (!method->isOverloadedOperator()) {
                    method_groups[method->getName()].push_back(method);
                }
            } break;
            case clang::Decl::Kind::CXXConstructor: {
                auto ctor = static_cast<const clang::CXXConstructorDecl*>(decl);
                constructors.push_back(ctor);
            } break;
            case clang::Decl::Kind::CXXDestructor: {
                destructor = static_cast<const clang::CXXDestructorDecl*>(decl);
            } break;
            case clang::Decl::Kind::CXXConversion: {
                auto conv = static_cast<const clang::CXXConversionDecl*>(decl);
                conversions.push_back(conv);
            } break;
            case clang::Decl::Kind::Field: {} break;
            default:
                decls.push_back(decl);
                break;
            }
        }

        for(const auto& [overloadGroupName, overloads]: method_groups) {
            if(overloads.size() == 1u) {
                const auto Method = overloads[0];
                const bool isCtor = Method->getKind() == clang::Decl::Kind::CXXConstructor;
                const bool overloadedOperator = Method->isOverloadedOperator();
                const bool isDtor = Method->getKind() == clang::Decl::Kind::CXXDestructor;
                const auto methodQualName = Method->getQualifiedNameAsString();

                if (!overloadedOperator && !isCtor && !isDtor) {
                    auto methodScope = ownScope.spawn(overloadGroupName, methodQualName, "mc::Method");
                    genMethodDispatcher(methodScope, Method, Record);
                    methodScope.putline("using return_type = {};", Method->getReturnType().getAsString(printingPolicy));

                    std::vector<clang::ParmVarDecl*> parameters(Method->parameters());
                    for (const auto param: parameters) {
                        auto parmName = param->getNameAsString();
                        auto paramScope = methodScope.spawn(parmName, parmName, "mc::Parameter");
                        methodScope.putline("using type = {};", param->getType().getAsString(printingPolicy));
                    }
                    wrap_range_in_tuple("parameters", methodScope.inner, parameters);
                    method_descriptors.emplace_back(fmt::format("meta_{}", Method->getNameAsString()));
                }
            } else {
                const bool isCtorGrp = overloadGroupName == name;
                const auto methodQualName = overloads[0]->getQualifiedNameAsString();
                auto overloadScope = ownScope.spawn(isCtorGrp ? "ctor" : overloadGroupName, methodQualName, "mc::OverloadSet");
                std::vector<std::string> overloadNames;

                for(const auto Method: overloads) {
                    const auto methodQualName = Method->getQualifiedNameAsString();
                    const auto overloadName = fmt::format("overload_{}", overloadNames.size());
                    auto methodScope = overloadScope.spawn(overloadName, methodQualName, "mc::Method");
                    methodScope.putline("using return_type = {};", Method->getReturnType().getAsString(printingPolicy));
                    genMethodDispatcher(methodScope, Method, Record);

                    std::vector<std::string> parameters;
                    for (const auto param: Method->parameters()) {
                        auto parmName = param->getNameAsString();
                        if (isCtorGrp) {
                            const auto ctorDecl = static_cast<const clang::CXXConstructorDecl*>(Method);
                            if(ctorDecl->isImplicit()) {
                                parmName = fmt::format("implicit_arg_{}", parameters.size());
                            }
                        }

                        auto paramScope = methodScope.spawn(parmName, parmName, "mc::Parameter");
                        paramScope.putline("using type = {};", param->getType().getAsString(printingPolicy));
                        parameters.emplace_back(fmt::format("meta_{}",parmName));
                    }
                    wrap_range_in_tuple("parameters", methodScope.inner, parameters);
                    overloadNames.emplace_back(fmt::format("meta_{}", overloadName));
                }
                wrap_range_in_tuple("overloads", overloadScope.inner, overloadNames);
                method_descriptors.emplace_back(fmt::format("meta_{}" , isCtorGrp ? "ctor" : overloadGroupName));
            }
        }

        wrap_range_in_tuple("methods", ownScope.inner, method_descriptors);
    }

    void ReflectionDataGenerator::exportEnum(const clang::EnumDecl *Enum, descriptor_scope &where) {
        auto qualName = Enum->getQualifiedNameAsString();
        auto name = Enum->getNameAsString();
        if (name.empty()) {
            name = Enum->getTypedefNameForAnonDecl()->getNameAsString();
            qualName = Enum->getTypedefNameForAnonDecl()->getQualifiedNameAsString();
        }

        auto ownScope = where.spawn(name, qualName, "mc::Enum");
        ownScope.putline("using type = {};", qualName);
        std::vector<clang::EnumConstantDecl*> enumerators;
        for(const auto enumerator: Enum->enumerators()) {
            auto enName = enumerator->getNameAsString();
            auto enScope = ownScope.spawn(enName, enumerator->getQualifiedNameAsString(), "mc::Enumerator");
            enScope.putline("static constexpr {} value = {};", Enum->getIntegerType().getTypePtrOrNull() ? Enum->getIntegerType().getAsString(printingPolicy) : "int",enumerator->getInitVal().toString(10));
            enumerators.push_back(enumerator);
        }

        wrap_range_in_tuple("enumerators", ownScope.inner, enumerators);
    }

    void ReflectionDataGenerator::exportNamespace(const clang::NamespaceDecl *Namespace, descriptor_scope &where) {
        auto qualName = Namespace->getQualifiedNameAsString();
        auto name = Namespace->getNameAsString();
        auto ownScope = where.spawn(name, qualName, "mc::Namespace");
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
