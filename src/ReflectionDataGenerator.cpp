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

namespace mc {
    namespace fs = std::experimental::filesystem;


    ReflectionDataGenerator::ReflectionDataGenerator(clang::ASTContext &astContext, clang::Sema &Sema)
        :out(mcOutput),
        idman(astContext.getPrintingPolicy()),
        idrepo(),
        context(astContext),
        sema(Sema),
        printingPolicy(astContext.getPrintingPolicy()) {

        global_scope.putline("#pragma once");
        global_scope.putline("#include <array>");
        global_scope.putline("#include <string_view>");

        global_scope.putline("#include <rosewood/rosewood.hpp>");
        global_scope.putline("#include <rosewood/type.hpp>");
        auto mainFile = astContext.getSourceManager().getMainFileID();
        auto mainFileLoc = astContext.getSourceManager().getComposedLoc(mainFile, 0);
        auto mainFilePath = astContext.getSourceManager().getFilename(mainFileLoc);

        global_scope.putline("#include \"{}\"", mainFilePath.str());
        global_scope.putline("");
        global_scope.putline("namespace rosewood {{");
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

    clang::QualType ReflectionDataGenerator::getUnitType(clang::QualType T) {
        auto type = T.getTypePtr();

        auto isNotUnitType = [] (auto type) {
            return type->isPointerType() || type->isReferenceType();
        };

        while (isNotUnitType(type)) {
            type = type->getPointeeType().getTypePtr();
        }

        return clang::QualType(type, 0);
    }


    void ReflectionDataGenerator::Generate() {
        printingPolicy = context.getPrintingPolicy();
        descriptor_scope module_scope = descriptor_scope(global_scope.spawn(), mcModuleName.getValue(), "rosewood::Module");

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
                        exportedClasses.push_back(fmt::format("meta_{}", exportCxxRecord(record->getNameAsString(), record, module_scope).name));
                    }
                } break;
                case clang::Decl::Kind::ClassTemplateSpecialization: {
                    // auto specialization = static_cast<clang::ClassTemplateSpecializationDecl*>(decl);
                    // this is just a test. it's not expected to work due to template naming
                    // if(specialization->isThisDeclarationADefinition()) {
                    //    exportedClasses.push_back(fmt::format("meta_{}", exportCxxRecord(specialization, module_scope).name));
                    // }
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
                return exportCxxRecord(record->getNameAsString(), record, where);
            }
        } break;
        default:
            break;
        }
        // TDO: Once we have better coverage of declaration types, this should become a throw
        return where.spawn("", "");
    }


    void getFastMethodDispatcher(descriptor_scope &descScope, const clang::CXXMethodDecl* Method, const clang::CXXRecordDecl *Record) {
        // this should be more elaborate and should consider all method attributes ( const and noexcept come to mind )
        descScope.putline("static inline void fastcall (");

        scope argList = descScope.inner.spawn();
        const std::size_t numArgs = Method->param_size();
        const auto &printingPolicy(Method->getASTContext().getPrintingPolicy());
        argList.putline("{}void *obj,", Method->isConst() ? "const ": "");
        argList.putline("[[maybe_unused]] void *retAddr,");
        argList.putline("[[maybe_unused]] void **args) {{");
        auto dispatcherBody = argList.spawn();
        const bool isCtor(Method->getKind() == clang::Decl::Kind::CXXConstructor);
        const bool isDtor(Method->getKind() == clang::Decl::Kind::CXXDestructor);
        const auto thisTypeName = clang::QualType(Record->getTypeForDecl(), 0).getAsString(printingPolicy);
        if (isCtor) {
            dispatcherBody.putline("new (obj) {} ({}", thisTypeName, numArgs > 0 ? "" : ");");
        } else if (isDtor) {
            dispatcherBody.putline("{1}{0} &Object = *reinterpret_cast<{1}{0}*>(obj);", thisTypeName, Method->isConst() ? "const ": "");
            dispatcherBody.putline("Object.~{}();", Record->getNameAsString());
        } else {
            dispatcherBody.putline("{1}{0} &Object = *reinterpret_cast<{1}{0}*>(obj);", thisTypeName, Method->isConst() ? "const ": "");
            const auto returnType = Method->getReturnType().getCanonicalType();
            const bool isReferenceType = returnType->isReferenceType() || returnType->isRValueReferenceType();
            const auto castType = returnType->isReferenceType() || returnType->isRValueReferenceType() ? returnType->getPointeeType() : returnType;
            const std::string retAddrCastExpr(fmt::format("*reinterpret_cast<{}{}*>(retAddr) = {}", castType.getAsString(printingPolicy), isReferenceType ? "*": "", isReferenceType? "&": ""));
            dispatcherBody.putline("{}Object.{} ({}", returnType->isVoidType() ? "" : retAddrCastExpr, Method->getNameAsString(), numArgs > 0 ? "" : ");");
        }

        auto callArgList = dispatcherBody.spawn();

        std::size_t index(0);
        for (const auto arg: Method->parameters()) {
            const auto argType = arg->getType();
            const bool isRValue = argType->isRValueReferenceType();
            const auto castType = argType->isReferenceType() || argType->isRValueReferenceType() ? argType->getPointeeType() : argType;
            const std::string paramCastExpr(fmt::format("*reinterpret_cast<{}*>(args[{}])", castType.getCanonicalType().getAsString(printingPolicy), index));
            callArgList.putline("{}{}{}{}", isRValue ? "std::move(" : "", paramCastExpr, isRValue ? ")": "" ,index < (numArgs - 1) ? ",": "");
            ++index;
        }
        if (numArgs > 0) {
            dispatcherBody.putline(");");
        }
        descScope.putline("}}");
    }

    void ReflectionDataGenerator::exportType(const std::string &exportAs, clang::QualType type, descriptor_scope &where) {
        const auto canonicalTypeName = type.getCanonicalType().getAsString(printingPolicy);
        const auto plainTypeName = type.getAsString(printingPolicy);
        const auto atomicTypeName = getUnitType(type).getCanonicalType().getAsString(printingPolicy);
        where.putline("static constexpr rosewood::Type<{0}> {3}{{\"{1}\", \"{0}\", \"{2}\"}};", canonicalTypeName, plainTypeName, atomicTypeName, exportAs);
    }

    descriptor_scope ReflectionDataGenerator::exportCxxMethod(const std::string &name, const clang::CXXRecordDecl *record, const clang::CXXMethodDecl* method, descriptor_scope &outerScope) {
        auto methodScope = outerScope.spawn(name, "rosewood::Method");
        getFastMethodDispatcher(methodScope, method, record);

        exportType("return_type", method->getReturnType(), methodScope);
        methodScope.putline("static constexpr bool is_const = {};", method->isConst());

        std::vector<std::string> parameters;
        for (const auto param: method->parameters()) {
            auto parmName = (param->getDeclName().isEmpty() || param->isImplicit() || method->isImplicit()) ? fmt::format("implicit_arg_{}", parameters.size()) : param->getNameAsString();
            auto paramScope = methodScope.spawn(parmName, "rosewood::Parameter");
            exportType("type", param->getType(), paramScope);
            parameters.emplace_back(fmt::format("meta_{}", parmName));
        }
        wrap_range_in_tuple("parameters", methodScope.inner, parameters);
        //

        //
        return methodScope;
    }

    std::string ReflectionDataGenerator::buildMethodSignature(const clang::CXXMethodDecl *method) {
        // build the argument list
        std::ostringstream sstream;
        auto functionPrototype = method->getType()->getAs<clang::FunctionProtoType>();
        auto exceptionSpecTempalte = functionPrototype->getExceptionSpecTemplate();
        auto exceptionSpecifier = functionPrototype->getExceptionSpecType();
        bool isNoExcept = false;
        if (method->getName() == "assign") {
            llvm::outs() << "";
        }
        switch (exceptionSpecifier) {
            case clang::ExceptionSpecificationType::EST_NoexceptTrue:
                [[fallthrough]];
            case clang::ExceptionSpecificationType::EST_BasicNoexcept:
                isNoExcept = true;
            break;
            default:
            break;
        }
        sstream << method->getReturnType().getCanonicalType().getAsString(printingPolicy);
        sstream << fmt::format(" ({}::*) (", clang::QualType(method->getParent()->getTypeForDecl(), 0).getAsString(printingPolicy));
        if (method->parameters().empty()) {

        } else {
            int paramIdx = 0;
            for (const auto& parm: method->parameters()) {
                sstream << parm->getType().getCanonicalType().getAsString(printingPolicy);
                sstream << (paramIdx < (method->parameters().size() - 1) ? ",": "");
                ++paramIdx;
            }
        }
        sstream << ")";
        sstream << fmt::format("{}{}",
                               functionPrototype->isConst() ? " const" : "",
                               isNoExcept ? " noexcept": "");

        auto res = sstream.str();
        return res;
    }

    descriptor_scope ReflectionDataGenerator::exportCxxMethodGroup(const std::string &name, const clang::CXXRecordDecl *Record, const std::vector<const clang::CXXMethodDecl*> &overloads, descriptor_scope &outerScope) {
        auto overloadScope = outerScope.spawn(name, "rosewood::OverloadSet");

        std::vector<std::string> overloadNames;

        int methodIndex = 0;
        for(const auto Method: overloads) {
            const auto overloadName = fmt::format("overload_{}", overloadNames.size());
            exportCxxMethod(overloadName, Record, Method, overloadScope);
            overloadNames.emplace_back(fmt::format("meta_{}", overloadName));


            outerScope.putline(fmt::format("static constexpr auto {}_{} = ", Method->getNameAsString(), methodIndex));
            outerScope.inner.increaseIndentation();
            outerScope.putline(fmt::format("rosewood::MethodDeclaration("));
            outerScope.inner.increaseIndentation();

            outerScope.putline(fmt::format("static_cast<{}>(&{}),", buildMethodSignature(Method), Method->getQualifiedNameAsString()));
            outerScope.putline(fmt::format("\"{}\",", Method->getNameAsString()));
            if (Method->parameters().empty()) {
                outerScope.putline("std::tuple<>());");
            } else {
                int paramIdx = 0;
                outerScope.putline("std::tuple(");
                outerScope.inner.increaseIndentation();
                for (const auto& param: Method->parameters()) {
                    outerScope.putline(fmt::format("rosewood::FunctionParameter<{}>(\"{}\", {}){}",
                                                   param->getType().getCanonicalType().getAsString(printingPolicy),
                                                   param->getNameAsString(),
                                                   param->hasDefaultArg(),
                                                   paramIdx < (Method->parameters().size() - 1) ? ",": ""
                                                   ));
                    ++paramIdx;
                }
                outerScope.inner.decreaseIndentation();
                outerScope.putline("));");
            }

            outerScope.inner.decreaseIndentation();outerScope.inner.decreaseIndentation();
            ++methodIndex;
        }
        wrap_range_in_tuple("overloads", overloadScope.inner, overloadNames);
        return overloadScope;
    }

    descriptor_scope ReflectionDataGenerator::exportCxxOperator(const std::string &name, const clang::CXXRecordDecl *record, const std::vector<const clang::CXXMethodDecl*> &overloads, descriptor_scope &outerScope) {
        auto opScope = outerScope.spawn(name, "rosewood::Operator");
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

    /*descriptor_scope ReflectionDataGenerator::exportCxxStaticOperator(const std::string &, const std::vector<const clang::FunctionDecl*> &, descriptor_scope &where) {
        return descriptor_scope(where.inner, "", "", "", "");
    }

    descriptor_scope ReflectionDataGenerator::exportFunctions(const std::string &, const std::vector<const clang::FunctionDecl*> &, descriptor_scope &where) {
        return descriptor_scope(where.inner, "", "", "", "");
    }*/

    descriptor_scope ReflectionDataGenerator::exportCxxConstructors(const std::vector<const clang::CXXConstructorDecl*> &overloads, const clang::CXXRecordDecl *record, descriptor_scope &outerScope) {
        static const std::string name = "constructor";
        auto overloadScope = outerScope.spawn(name, "rosewood::ConstructorSet");

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
            auto fieldScope = outerScope.spawn(field->getNameAsString(), "rosewood::Field");
            printingPolicy.FullyQualifiedName = true;
            exportType("type", field->getType(), fieldScope);
        }
    }

    bool ReflectionDataGenerator::areMethodArgumentsPubliclyUsable(const clang::CXXMethodDecl* method) {
        for (const auto& param: method->parameters()) {
            auto parmType = param->getType();
            if (parmType->isRecordType()) {
                auto record = parmType->getAsRecordDecl();
                if (!record->getAccess() == clang::AccessSpecifier::AS_public) {
                    return false;
                }
            } // this will likely need to be extended to cover more cases
        }
        auto functionPrototype = method->getType()->getAs<clang::FunctionProtoType>();
        auto exceptionSpecKind = functionPrototype->getExceptionSpecType();
        // cannot export a function if we cannot reliably determine it's exception specification
        // since that's part of the c++17 type system
        return  exceptionSpecKind != clang::ExceptionSpecificationType::EST_Uninstantiated &&
                exceptionSpecKind != clang::ExceptionSpecificationType::EST_DependentNoexcept;
    }


    descriptor_scope ReflectionDataGenerator::exportCxxRecord(const std::string &name, const clang::CXXRecordDecl *Record, descriptor_scope &where) {
        auto ownScope = where.spawn(name, "rosewood::Class");
        ownScope.putline("using type = {};", clang::QualType(Record->getTypeForDecl(), 0).getAsString(printingPolicy));
        ownScope.putline("static constexpr std::string_view qualified_name = \"{}\";", Record->getQualifiedNameAsString());
        std::map<std::string_view, std::set<std::string>> descriptornames = {
            {"classes", {}},
            {"overload_sets", {}},
            {"operators", {}}, // not 100% these shouldn't just be treated as any other methods
            {"fields", {}},
            {"enums", {}}
        };

        // in order to do overloading we need to do three passes over all methods
        std::map<std::string, std::vector<const clang::CXXMethodDecl*>> method_groups;
        std::map<std::string, std::vector<const clang::CXXMethodDecl*>> overloaded_operators;
        // std::map<llvm::StringRef, std::vector<const clang::FunctionDecl*>> function_groups;
        // std::map<std::string, std::vector<const clang::FunctionDecl*>> static_operator_groups;
        std::vector<const clang::CXXConstructorDecl*> constructors;
        std::vector<const clang::CXXConversionDecl*> conversions;
        std::vector<const clang::FieldDecl*> fields;
        std::vector<const clang::CXXRecordDecl*> classes;
        std::vector<const clang::EnumDecl*> enums;

        // std::vector<const clang::Decl*> decls;
        const clang::CXXDestructorDecl *destructor = nullptr;

        for(const auto decl: Record->decls()) {
            // first trim out non public members
            if(decl->getAccess() != clang::AccessSpecifier::AS_public) continue;
            switch (decl->getKind()) {
            case clang::Decl::Kind::CXXMethod: {
                auto method = static_cast<const clang::CXXMethodDecl*>(decl);
                if (!areMethodArgumentsPubliclyUsable(method)) continue;
                // TDO: the other half
                if (!method->isOverloadedOperator()) {
                    const auto methodName = method->getNameAsString();
                    method_groups[methodName].push_back(method);
                    descriptornames["overload_sets"].insert(fmt::format("meta_{}", methodName));
                } else {
                    const auto opName = fmt::format("operator_{}", overloaded_operators.size());
                    overloaded_operators[opName].push_back(method);
                    descriptornames["operators"].insert(fmt::format("meta_{}", opName));
                }
            } break;
            case clang::Decl::Kind::CXXConstructor: {
                auto ctor = static_cast<const clang::CXXConstructorDecl*>(decl);
                if (!areMethodArgumentsPubliclyUsable(ctor)) continue;
                if(!ctor->isDeleted()) {
                    constructors.push_back(ctor);
                }
            } break;
            case clang::Decl::Kind::CXXDestructor: {
                destructor = static_cast<const clang::CXXDestructorDecl*>(decl);
            } break;
            case clang::Decl::Kind::CXXConversion: {
                auto conv = static_cast<const clang::CXXConversionDecl*>(decl);
                if (!areMethodArgumentsPubliclyUsable(conv)) continue;
                conversions.push_back(conv);
            } break;
            case clang::Decl::Kind::Field: {
                auto field = static_cast<const clang::FieldDecl*>(decl);
                fields.push_back(field);
                descriptornames["fields"].insert(fmt::format("meta_{}", field->getNameAsString()));
            } break;
            case clang::Decl::Kind::CXXRecord: {
                auto cls = static_cast<const clang::CXXRecordDecl*>(decl);
                if (cls->isThisDeclarationADefinition()) {
                    classes.push_back(cls);
                }
            } break;
            case clang::Decl::Kind::Enum: {
                auto en = static_cast<const clang::EnumDecl*>(decl);
                if (en->isThisDeclarationADefinition()) {
                    enums.push_back(en);
                }
            } break;

            default:
                break;
            }
        }

        if (constructors.size()) {
            exportCxxConstructors(constructors, Record, ownScope);
        }
        for(const auto &[grpName, grp]: method_groups) {
            exportCxxMethodGroup(grpName, Record, grp, ownScope);
        }

        for(const auto &[opName, opGrp]: overloaded_operators) {
            exportCxxOperator(opName, Record, opGrp, ownScope);
        }

        exportFields(fields, ownScope);
        if (destructor != nullptr) {
            exportCxxDestructor(destructor, Record, ownScope);
        }

        for(const auto cls: classes) {
            auto exportedScope = exportCxxRecord(cls->getNameAsString(), cls, ownScope);
            descriptornames["classes"].emplace(fmt::format("meta_{}", exportedScope.name));
        }
        for(const auto en: enums) {
            auto exportedScope = exportEnum(en, ownScope);
            descriptornames["enums"].emplace(fmt::format("meta_{}", exportedScope.name));
        }

        for(const auto& [rangeName, range]: descriptornames) {
            wrap_range_in_tuple(rangeName, ownScope.inner, range);
        }

        exportedMetaTypes.emplace_back(std::tuple(clang::QualType(Record->getTypeForDecl(),0).getAsString(printingPolicy), ownScope.qualifiedName));
        return ownScope;
    }

    descriptor_scope ReflectionDataGenerator::exportEnum(const clang::EnumDecl *Enum, descriptor_scope &where) {
        auto qualName = Enum->getQualifiedNameAsString();
        auto name = Enum->getNameAsString();
        if (name.empty()) {
            name = Enum->getTypedefNameForAnonDecl()->getNameAsString();
            qualName = Enum->getTypedefNameForAnonDecl()->getQualifiedNameAsString();
        }

        auto ownScope = where.spawn(name, "rosewood::Enum");
        ownScope.putline("using type = {};", qualName);
        ownScope.putline("using enumerator_type = Enumerator<{}>;", Enum->getIntegerType().getAsString(printingPolicy));
        std::vector<clang::EnumConstantDecl*> enumerators(Enum->enumerators().begin(), Enum->enumerators().end());
        ownScope.putline("static constexpr std::array<Enumerator<{}>, {}> enumerators {{", Enum->getIntegerType().getAsString(printingPolicy), enumerators.size());

        for(unsigned index(0); index < enumerators.size(); ++index) {
            auto enumerator = enumerators[index];
            auto enName = enumerator->getNameAsString();
            auto enScope = ownScope.inner.spawn();
            enScope.putline("Enumerator<{}> {{ {}, \"{}\" }}{}", Enum->getIntegerType().getAsString(printingPolicy), enumerator->getInitVal().toString(10), enName, index < (enumerators.size() - 1) ? ",": std::string());
        }

        ownScope.putline("}};");
        exportedMetaTypes.emplace_back(std::tuple(qualName, ownScope.qualifiedName));
        return ownScope;
    }

    descriptor_scope ReflectionDataGenerator::exportNamespace(const clang::NamespaceDecl *Namespace, descriptor_scope &where) {
        auto qualName = Namespace->getQualifiedNameAsString();
        auto name = Namespace->getNameAsString();
        auto ownScope = where.spawn(name, "rosewood::Namespace");

        ownScope.print_header();

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
                    exportedClasses.push_back(fmt::format("meta_{}", exportCxxRecord(record->getNameAsString()  , record, ownScope).name));
                }
            } break;
            case clang::Decl::TypeAlias: {
                auto alias = static_cast<clang::TypeAliasDecl*>(decl);
                auto aliasedType = alias->getUnderlyingType();
                if (aliasedType->isRecordType()) {
                    auto record = aliasedType->getAsCXXRecordDecl();
                    if (record->getKind() == clang::Decl::Kind::ClassTemplateSpecialization) {
                        auto specialization = static_cast<clang::ClassTemplateSpecializationDecl*>(record);
                        sema.RequireCompleteType(alias->getLocation(), clang::QualType(specialization->getTypeForDecl(), 0), 1);
                        exportedClasses.push_back(fmt::format("meta_{}", exportCxxRecord(alias->getNameAsString(), specialization->getDefinition(), ownScope).name));
                    }
                }
            };
            default:
                break;
            }
        }
        wrap_range_in_tuple("namespaces", ownScope.inner, exportedNamespaces);
        wrap_range_in_tuple("enums", ownScope.inner, exportedEnums);
        wrap_range_in_tuple("classes", ownScope.inner, exportedClasses);

        return ownScope;
    }

}
