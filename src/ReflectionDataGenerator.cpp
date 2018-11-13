#include "ReflectionDataGenerator.h"

#include <unordered_map>
#include <string_view>
#include <map>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <experimental/filesystem>
#include <clang/Basic/OperatorKinds.h>

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

        global_scope.putline("#include \"{}\"", mainFilePath.str());
        global_scope.putline("");
        global_scope.putline("namespace mc {{");
    }

    ReflectionDataGenerator::~ReflectionDataGenerator() {
        for(const auto &[declName, descriptorName]: exportedMetaTypes) {
            global_scope.putline("template <>");
            global_scope.putline("struct meta <{}> : public {} {{}};", declName, descriptorName);
        }

        global_scope.putline("}}");
        idrepo.save(mcJsonOutput.getValue());
        out.flush();
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

        std::size_t idx(0);
        for(const auto& item: range) {
            initScope.indent();

            if constexpr (std::is_same<typename declRangeT::value_type, std::string>::value) {
                initScope.rawput(item);
            } else {
                auto itemName = item->getNameAsString();
                initScope.rawput("meta_{}", itemName);
            }

            if (idx < (numElements - 1)) {
                initScope.rawput(",");
            }

            initScope.rawput("\n");
            ++idx;
        }
        where.putline(">;");
    }

    void ReflectionDataGenerator::Generate() {
        printingPolicy = context.getPrintingPolicy();
        descriptor_scope module_scope = descriptor_scope(global_scope.spawn(), mcModuleName.getValue(), mcModuleName.getValue(), "mc::Module");

        std::vector<std::string> exportedNamespaces;
        std::vector<std::string> exportedEnums;
        std::vector<std::string> exportedClasses;

        for(const auto decl: context.getTranslationUnitDecl()->decls()) {
            // first cull out everything that isn't defined within the `main` file
            const bool inMainFile(context.getSourceManager().isInMainFile(decl->getLocation()));
            if (inMainFile) {
                switch(auto declKind = decl->getKind()) {
                case clang::Decl::Kind::Namespace:
                    exportedNamespaces.push_back(fmt::format("meta_{}", exportNamespace(static_cast<const clang::NamespaceDecl*>(decl), module_scope).name));
                    break;
                case clang::Decl::Kind::Enum:
                    exportedEnums.push_back(fmt::format("meta_{}", exportEnum(static_cast<const clang::EnumDecl*>(decl), module_scope).name));
                    break;
                case clang::Decl::Kind::CXXRecord: {
                    auto record = static_cast<const clang::CXXRecordDecl*>(decl);
                    if (record->isThisDeclarationADefinition()) {
                        exportedClasses.push_back(fmt::format("meta_{}", exportCxxRecord(static_cast<const clang::CXXRecordDecl*>(decl), module_scope).name));
                    }
                } break;
                default:
                    // report?
                    break;
                }
            }


        }
        wrap_range_in_tuple("namespaces", module_scope.inner, exportedNamespaces);
        wrap_range_in_tuple("enums", module_scope.inner, exportedEnums);
        wrap_range_in_tuple("classes", module_scope.inner, exportedClasses);
    }

    descriptor_scope ReflectionDataGenerator::exportDeclaration(const clang::Decl *Decl, descriptor_scope &where) {
        // so this is a sort of a type dispatcher
        switch(auto declKind = Decl->getKind()) {
        case clang::Decl::Kind::Namespace:
            return exportNamespace(static_cast<const clang::NamespaceDecl*>(Decl), where);
        case clang::Decl::Kind::Enum:
            return exportEnum(static_cast<const clang::EnumDecl*>(Decl), where);
        case clang::Decl::Kind::CXXRecord: {
            auto record = static_cast<const clang::CXXRecordDecl*>(Decl);
            if (record->isThisDeclarationADefinition()) {
                return exportCxxRecord(static_cast<const clang::CXXRecordDecl*>(Decl), where);
            }
        } break;
        default:
            break;
        }
        // TDO: Once we have better coverage of declaration types, this should become a throw
        return where.spawn("", "", "");
    }


    void genMethodDispatcher(descriptor_scope &descScope, const clang::CXXMethodDecl* Method, const clang::CXXRecordDecl *Record) {
        // this should be more elaborate and should consider all method attributes ( is const for example )
        descScope.putline("static constexpr auto call = [] (");

        scope argList = descScope.inner.spawn();
        const std::size_t numArgs = Method->param_size();
        const bool isConstructor = Method->getKind() == clang::Decl::Kind::CXXConstructor;
        const bool isImplicit = Method->isImplicit();
        if (isConstructor) {
            argList.putline("void *addr{}", numArgs > 0 ? ",": ") {");
        } else {
            argList.putline("{}{}& obj{}", Method->isConst() ? "const ": "", Record->getQualifiedNameAsString(), numArgs > 0 ? ",": ") {");
        }

        std::size_t index(0);
        for (const auto arg: Method->parameters()) {
            argList.putline("{} {}{}", arg->getType().getAsString(Method->getASTContext().getPrintingPolicy()),  isImplicit ? fmt::format("implicit_arg_{}", index) : arg->getNameAsString(), index < (numArgs - 1) ? ",": ") {");
            ++index;
        }
        auto lambdaBody = argList.spawn();

        if (isConstructor) {
            lambdaBody.putline("new (addr) {} {}", Record->getQualifiedNameAsString(), numArgs > 0 ? "(": ";");
        } else {
            lambdaBody.putline("return obj.{} ({}", Method->getNameAsString(), numArgs > 0 ? "" : ");");
        }
        auto callArgList = lambdaBody.spawn();
        index = 0;
        for (const auto arg: Method->parameters()) {
            if (arg->getType()->isRValueReferenceType()) {
                callArgList.putline("std::move({}){}", isImplicit ? fmt::format("implicit_arg_{}", index) : arg->getNameAsString(), index < (numArgs - 1) ? ",": "");
            } else {
                callArgList.putline("{}{}", isImplicit ? fmt::format("implicit_arg_{}", index) : arg->getNameAsString(), index < (numArgs - 1) ? ",": "");
            }
            ++index;
        }
        if (numArgs > 0) {
            lambdaBody.putline(");");
        }
        descScope.putline("}};");
    }


    descriptor_scope ReflectionDataGenerator::exportCxxMethod(const std::string &name, const clang::CXXRecordDecl *record, const clang::CXXMethodDecl* method, descriptor_scope &outerScope) {
        const auto methodQualName = method->getQualifiedNameAsString();

        auto methodScope = outerScope.spawn(name, methodQualName, "mc::Method");
        genMethodDispatcher(methodScope, method, record);
        methodScope.putline("using return_type = {};", method->getReturnType().getAsString(printingPolicy));

        std::vector<std::string> parameters;
        for (const auto param: method->parameters()) {
            auto parmName = (param->isImplicit() || method->isImplicit()) ? fmt::format("implicit_arg_{}", parameters.size()) : param->getNameAsString();
            auto paramScope = methodScope.spawn(parmName, parmName, "mc::Parameter");
            paramScope.putline("using type = {};", param->getType().getAsString(printingPolicy));
            parameters.emplace_back(fmt::format("meta_{}", parmName));
        }
        wrap_range_in_tuple("parameters", methodScope.inner, parameters);
        return methodScope;
    }

    descriptor_scope ReflectionDataGenerator::exportCxxMethodGroup(const std::string &name, const clang::CXXRecordDecl *Record, const std::vector<const clang::CXXMethodDecl*> &overloads, descriptor_scope &outerScope) {
        // the unwanted methods have already been filtered out at this point so it's just a straightforward export
        const auto methodQualName = overloads[0]->getQualifiedNameAsString();
        auto overloadScope = outerScope.spawn(name, methodQualName, "mc::OverloadSet");

        std::vector<std::string> overloadNames;

        for(const auto Method: overloads) {
            const auto overloadName = fmt::format("overload_{}", overloadNames.size());
            exportCxxMethod(overloadName, Record, Method, overloadScope);
            overloadNames.emplace_back(fmt::format("meta_{}", overloadName));
        }
        wrap_range_in_tuple("overloads", overloadScope.inner, overloadNames);
        return overloadScope;
    }

    descriptor_scope ReflectionDataGenerator::exportCxxOperator(const std::string &name, const clang::CXXRecordDecl *record, const std::vector<const clang::CXXMethodDecl*> &overloads, descriptor_scope &outerScope) {
        const auto qualName = overloads[0]->getQualifiedNameAsString();
        auto opScope = outerScope.spawn(name, qualName, "mc::Operator");

        std::vector<std::string> overloadNames;

        for(const auto Method: overloads) {
            const auto overloadName = fmt::format("overload_{}", overloadNames.size());
            auto overloadScope = exportCxxMethod(overloadName, record, Method, opScope);
            const auto spelling = clang::getOperatorSpelling(Method->getOverloadedOperator());
            overloadScope.putline("static constexpr std::string_view spelling = \"{}\";", spelling);
            overloadNames.emplace_back(fmt::format("meta_{}", overloadName));
        }
        wrap_range_in_tuple("overloads", opScope.inner, overloadNames);

        return opScope;
    }

    descriptor_scope ReflectionDataGenerator::exportCxxStaticOperator(const std::string &name, const std::vector<const clang::FunctionDecl*> &overloads, descriptor_scope &where) {

    }

    descriptor_scope ReflectionDataGenerator::exportFunctions(const std::string &name, const std::vector<const clang::FunctionDecl*> &overloads, descriptor_scope &where) {

    }

    descriptor_scope ReflectionDataGenerator::exportCxxConstructors(const std::vector<const clang::CXXConstructorDecl*> &overloads, const clang::CXXRecordDecl *record, descriptor_scope &outerScope) {
        const auto methodQualName = overloads[0]->getQualifiedNameAsString();
        static const std::string name = "constructor";
        auto overloadScope = outerScope.spawn(name, methodQualName, "mc::ConstructorSet");

        std::vector<std::string> overloadNames;

        for(const auto Method: overloads) {
            const auto overloadName = fmt::format("overload_{}", overloadNames.size());
            exportCxxMethod(overloadName, record, Method, overloadScope);
            overloadNames.emplace_back(fmt::format("meta_{}", overloadName));
        }
        wrap_range_in_tuple("overloads", overloadScope.inner, overloadNames);
        return overloadScope;
    }

    descriptor_scope ReflectionDataGenerator::exportCxxDestructor(const clang::CXXDestructorDecl *Dtor, const clang::CXXRecordDecl *record, descriptor_scope &outerScope) {
        return exportCxxMethod("destructor", record, Dtor, outerScope);
    }

    void ReflectionDataGenerator::exportFields(const std::vector<const clang::FieldDecl*> &fields, descriptor_scope &outerScope) {
        for(const auto& field: fields) {
            auto fieldScope = outerScope.spawn(field->getNameAsString(), field->getQualifiedNameAsString(), "mc::Field");
            fieldScope.putline("using type = {};", field->getType().getAsString(printingPolicy));
        }
    }


    descriptor_scope ReflectionDataGenerator::exportCxxRecord(const clang::CXXRecordDecl *Record, descriptor_scope &where) {
        auto qualName = Record->getQualifiedNameAsString();
        auto name = Record->getNameAsString();
        auto ownScope = where.spawn(name, qualName, "mc::Class");
        ownScope.putline("using type = {};", Record->getQualifiedNameAsString());
        std::map<std::string_view, std::set<std::string>> descriptornames;

        // in order to do overloading we need to do three passes over all methods
        std::map<std::string, std::vector<const clang::CXXMethodDecl*>> method_groups;
        std::map<std::string, std::vector<const clang::CXXMethodDecl*>> overloaded_operators;
        // std::map<llvm::StringRef, std::vector<const clang::FunctionDecl*>> function_groups;
        // std::map<std::string, std::vector<const clang::FunctionDecl*>> static_operator_groups;
        std::vector<const clang::CXXConstructorDecl*> constructors;
        std::vector<const clang::CXXConversionDecl*> conversions;
        std::vector<const clang::FieldDecl*> fields;
        std::vector<const clang::Decl*> decls;
        const clang::CXXDestructorDecl *destructor = nullptr;

        for(const auto decl: Record->decls()) {
            // first trim out non public members
            if(decl->getAccess() != clang::AccessSpecifier::AS_public) continue;
            switch (decl->getKind()) {
            case clang::Decl::Kind::CXXMethod: {
                auto method = static_cast<const clang::CXXMethodDecl*>(decl);
                // TDO: the other half
                if (!method->isOverloadedOperator()) {
                    const auto methodName = method->getNameAsString();
                    method_groups[methodName].push_back(method);
                    descriptornames["methods"].insert(fmt::format("meta_{}", methodName));
                } else {
                    const auto opName = fmt::format("operator_{}", overloaded_operators.size());
                    overloaded_operators[opName].push_back(method);
                    descriptornames["operators"].insert(fmt::format("meta_{}", opName));
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
            case clang::Decl::Kind::Field: {
                auto field = static_cast<const clang::FieldDecl*>(decl);
                fields.push_back(field);
                descriptornames["fields"].insert(fmt::format("meta_{}", field->getNameAsString()));
            } break;
            default:
                decls.push_back(decl);
                break;
            }
        }

        exportCxxConstructors(constructors, Record, ownScope);
        for(const auto &[grpName, grp]: method_groups) {
            exportCxxMethodGroup(grpName, Record, grp, ownScope);
        }

        for(const auto &[opName, opGrp]: overloaded_operators) {
            exportCxxOperator(opName, Record, opGrp, ownScope);
        }

        exportFields(fields, ownScope);
        if (destructor != nullptr) exportCxxDestructor(destructor, Record, ownScope);

        for(const auto& [rangeName, range]: descriptornames) {
            wrap_range_in_tuple(rangeName, ownScope.inner, range);
        }

        std::vector<std::string> genDeclRange;
        for(const auto decl: decls) {
            auto exportedScope = exportDeclaration(decl, ownScope);
            if (!exportedScope.name.empty()) {
                genDeclRange.emplace_back(fmt::format("meta_{}", exportedScope.name));
            }
        }

        exportedMetaTypes.emplace_back(std::tuple(qualName, ownScope.qualifiedName));
        wrap_range_in_tuple("decls", ownScope.inner, genDeclRange);
        return ownScope;
    }

    descriptor_scope ReflectionDataGenerator::exportEnum(const clang::EnumDecl *Enum, descriptor_scope &where) {
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

        exportedMetaTypes.emplace_back(std::tuple(qualName, ownScope.qualifiedName));
        wrap_range_in_tuple("enumerators", ownScope.inner, enumerators);
        return ownScope;
    }

    descriptor_scope ReflectionDataGenerator::exportNamespace(const clang::NamespaceDecl *Namespace, descriptor_scope &where) {
        auto qualName = Namespace->getQualifiedNameAsString();
        auto name = Namespace->getNameAsString();
        auto ownScope = where.spawn(name, qualName, "mc::Namespace");


        std::vector<std::string> exportedNamespaces;
        std::vector<std::string> exportedEnums;
        std::vector<std::string> exportedClasses;

        for(const auto decl: Namespace->decls()) {
            switch(auto declKind = decl->getKind()) {
            case clang::Decl::Kind::Namespace:
                exportedNamespaces.push_back(fmt::format("meta_{}", exportNamespace(static_cast<const clang::NamespaceDecl*>(decl), ownScope).name));
                break;
            case clang::Decl::Kind::Enum:
                exportedEnums.push_back(fmt::format("meta_{}", exportEnum(static_cast<const clang::EnumDecl*>(decl), ownScope).name));
                break;
            case clang::Decl::Kind::CXXRecord: {
                auto record = static_cast<const clang::CXXRecordDecl*>(decl);
                if (record->isThisDeclarationADefinition()) {
                    exportedClasses.push_back(fmt::format("meta_{}", exportCxxRecord(static_cast<const clang::CXXRecordDecl*>(decl), ownScope).name));
                }
            } break;
            default:
                // report?
                break;
            }
        }
        wrap_range_in_tuple("namespaces", ownScope.inner, exportedNamespaces);
        wrap_range_in_tuple("enums", ownScope.inner, exportedEnums);
        wrap_range_in_tuple("classes", ownScope.inner, exportedClasses);

        return ownScope;
    }

}
