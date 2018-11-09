#include "RuntimeReflectionDataSourceGenerator.h"

#include <fmt/ostream.h>
#include <fmt/format.h>

#pragma warning(push, 0)
#include <clang/AST/AST.h>
#include <clang/AST/Type.h>
#pragma warning(pop)

#include <vector>
#include <map>

#include "SourceElements.h"


namespace mc {
/*
    void RuntimeReflectionDataSourceGenerator::BeginFile() {
    }

    void RuntimeReflectionDataSourceGenerator::EndFile() {

    }

    void RuntimeReflectionDataSourceGenerator::process(const clang::CXXRecordDecl *record) {

        std::vector<const clang::CXXMethodDecl*> exportedMethods;
        for (const auto &decl : record->decls()) {
            if (decl->getKind() == clang::Decl::Kind::CXXMethod) {
                // generate a packed call dispatched (unpacker)
                auto method = static_cast<const clang::CXXMethodDecl*>(decl);
                if (method->getAccess() == clang::AccessSpecifier::AS_public) {
                    // generate a method descriptor
                    const unsigned argc(method->getNumParams());
                    genTypeDescriptor(method->getReturnType().getTypePtr());
                    for (unsigned idx(0); idx < argc; ++idx) {
                        auto parm(method->getParamDecl(idx));
                        genTypeDescriptor(parm->getType().getTypePtr());
                    }

                    genMethodCallUnpacker(method);
                    genMethodDescriptor(method);
                    exportedMethods.push_back(method);
                }
            }
        }
        genRecordDescriptor(record, exportedMethods);
        genRecordDescBindingToVirtualFcn(record);
    }

    void RuntimeReflectionDataSourceGenerator::process(const clang::EnumDecl *en) {

    }

    void RuntimeReflectionDataSourceGenerator::process(const clang::FunctionDecl *fct) {

    }

    void RuntimeReflectionDataSourceGenerator::genTypeDescriptor(const clang::Type *ty) {
        auto identifier = genIdentifier(ty);
        if (isDefined(identifier)) return;

        switch (ty->getTypeClass()) {
        case clang::Type::TypeClass::Builtin: {
            
            auto btIn = static_cast<const clang::BuiltinType*>(ty);
            auto res = builtinTypes.find(btIn->getKind());
            assert(res != builtinTypes.end());

            source.addExpression(std::make_shared<mc::VariableDefinition>("const metal::BuiltinType", identifier, 
                std::vector<std::shared_ptr<Expression>>{
                    std::make_shared<mc::LiteralExpression>(std::string("metal::BuiltinType::BuiltinKind::") + res->second),
                    std::make_shared<mc::LiteralExpression>(std::string("\"") + btIn->getCanonicalTypeInternal().getAsString(printingPolicy) + std::string("\""))
                }
            ));
        } break;

        case clang::Type::TypeClass::Pointer: {
            auto pTy = static_cast<const clang::PointerType *>(ty);
            auto pointee = pTy->getPointeeType();
            auto pteeIdent(genIdentifier(pointee.getTypePtr()));
            if (!isDefined(pteeIdent))
                genTypeDescriptor(pointee.getTypePtr());
            
            source.addExpression(std::make_shared<mc::VariableDefinition>("const metal::PointerType", identifier, 
                std::vector<std::shared_ptr<Expression>>{
                    std::make_shared<mc::AnonymousVariable>(std::string("metal::QualType"), std::vector<std::shared_ptr<Expression>>{
                        std::make_shared<mc::BoolLiteral>(pointee.isConstQualified()),
                        std::make_shared<mc::BoolLiteral>(pointee.isVolatileQualified()),
                        std::make_shared<mc::BoolLiteral>(pointee.isRestrictQualified()),
                        std::make_shared<mc::AddressOf>(pteeIdent)
                    }),
                    std::make_shared<mc::CStringLiteral>(pointee.getAsString(printingPolicy) + "*")
                }
            ));

        } break;
        case clang::Type::TypeClass::LValueReference: {
            auto pTy = static_cast<const clang::LValueReferenceType *>(ty);
            auto pointee = pTy->getPointeeType();
            auto pteeIdent(genIdentifier(pointee.getTypePtr()));
            if (!isDefined(pteeIdent))
                genTypeDescriptor(pointee.getTypePtr());

            source.addExpression(std::make_shared<mc::VariableDefinition>("const metal::LReferenceType", identifier,
                std::vector<std::shared_ptr<Expression>>{
                    std::make_shared<mc::AnonymousVariable>(std::string("metal::QualType"), std::vector<std::shared_ptr<Expression>>{
                        std::make_shared<mc::BoolLiteral>(pointee.isConstQualified()),
                            std::make_shared<mc::BoolLiteral>(pointee.isVolatileQualified()),
                            std::make_shared<mc::BoolLiteral>(pointee.isRestrictQualified()),
                            std::make_shared<mc::AddressOf>(pteeIdent)
                    }),
                    std::make_shared<mc::CStringLiteral>(pointee.getAsString(printingPolicy) + "&")
                }
            ));

        } break;
        case clang::Type::TypeClass::RValueReference: {
            auto pTy = static_cast<const clang::RValueReferenceType *>(ty);
            auto pointee = pTy->getPointeeType();
            auto pteeIdent(genIdentifier(pointee.getTypePtr()));
            if (!isDefined(pteeIdent))
                genTypeDescriptor(pointee.getTypePtr());

            source.addExpression(std::make_shared<mc::VariableDefinition>("const metal::RReferenceType", identifier,
                std::vector<std::shared_ptr<Expression>>{
                std::make_shared<mc::AnonymousVariable>(std::string("metal::QualType"), std::vector<std::shared_ptr<Expression>>{
                    std::make_shared<mc::BoolLiteral>(pointee.isConstQualified()),
                        std::make_shared<mc::BoolLiteral>(pointee.isVolatileQualified()),
                        std::make_shared<mc::BoolLiteral>(pointee.isRestrictQualified()),
                        std::make_shared<mc::AddressOf>(pteeIdent)
                    }),
                    std::make_shared<mc::CStringLiteral>(pointee.getAsString(printingPolicy) + "&&")
                }
            ));


        } break;

        case clang::Type::TypeClass::Typedef: {
            auto tty = static_cast<const clang::TypedefType*>(ty);
            auto tdecl = tty->getDecl();
            auto ttIdent(genIdentifier(tdecl->getUnderlyingType().getTypePtr()));
            if (!isDefined(ttIdent)) 
                genTypeDescriptor(tdecl->getUnderlyingType().getTypePtr());

            source.addExpression(std::make_shared<mc::VariableDefinition>("const metal::TypedefType", identifier,
                std::vector<std::shared_ptr<Expression>>{
                std::make_shared<mc::AnonymousVariable>(std::string("metal::QualType"), std::vector<std::shared_ptr<Expression>>{
                    std::make_shared<mc::BoolLiteral>(tdecl->getUnderlyingType().isConstQualified()),
                        std::make_shared<mc::BoolLiteral>(tdecl->getUnderlyingType().isVolatileQualified()),
                        std::make_shared<mc::BoolLiteral>(tdecl->getUnderlyingType().isRestrictQualified()),
                        std::make_shared<mc::AddressOf>(ttIdent)
                    }),
                    std::make_shared<mc::CStringLiteral>(tdecl->getQualifiedNameAsString())
                }
            ));


        } break;
        case clang::Type::TypeClass::Elaborated: {
            auto ety = static_cast<const clang::ElaboratedType*>(ty);
            
            auto ttIdent(genIdentifier(ety->getNamedType().getTypePtr()));
            if (!isDefined(ttIdent))
                genTypeDescriptor(ety->getNamedType().getTypePtr());

            source.addExpression(std::make_shared<mc::VariableDefinition>("const metal::TypedefType", identifier,
                std::vector<std::shared_ptr<Expression>>{
                std::make_shared<mc::AnonymousVariable>(std::string("metal::QualType"), std::vector<std::shared_ptr<Expression>>{
                    std::make_shared<mc::BoolLiteral>(ety->getNamedType().isConstQualified()),
                        std::make_shared<mc::BoolLiteral>(ety->getNamedType().isVolatileQualified()),
                        std::make_shared<mc::BoolLiteral>(ety->getNamedType().isRestrictQualified()),
                        std::make_shared<mc::AddressOf>(ttIdent)
                    }),
                    std::make_shared<mc::CStringLiteral>(ety->getNamedType().getAsString(printingPolicy))
                }
            ));


        } break;
        case clang::Type::TypeClass::TemplateSpecialization: {
            auto tty = static_cast<const clang::TemplateSpecializationType*>(ty);
            auto decl = tty->getAsCXXRecordDecl();
            llvm::SmallVector<char, 255> ttt;
            llvm::raw_svector_ostream ot(ttt);
            // clang::QualType::print(ty, clang::Qualifiers(), ot, printingPolicy, llvm::Twine());
            // tty->getTemplateName().print(ot, printingPolicy);
            
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

            source.addExpression(std::make_shared<mc::VariableDefinition>("const metal::RecordType", identifier,
                std::vector<std::shared_ptr<Expression>>{
                std::make_shared<mc::AddressOf>(genIdentifier(static_cast<const clang::Decl *>(tty->getAsCXXRecordDecl()))),
                    std::make_shared<mc::CStringLiteral>(ot.str().str())
                }
            ));
            


        } break;
        case clang::Type::TypeClass::Record: {
            auto tty = static_cast<const clang::RecordType*>(ty);

            source.addExpression(std::make_shared<mc::VariableDefinition>("const metal::RecordType", identifier,
                std::vector<std::shared_ptr<Expression>>{
                    std::make_shared<mc::AddressOf>(genIdentifier(static_cast<const clang::Decl *>(tty->getAsCXXRecordDecl()))),
                    std::make_shared<mc::CStringLiteral>(tty->getDecl()->getQualifiedNameAsString())
                }
            ));


        } break;
        default:
            break;
        }
        out << std::endl << std::endl;

        defineIdentifier(identifier);
    }

    void RuntimeReflectionDataSourceGenerator::genMethodCallUnpacker(const clang::CXXMethodDecl *method) {
        auto identifier(genIdentifier(static_cast<const clang::Decl*>(method)));
        auto sres = fmt::format(
                    "void metal::InvokableDispatcher<{0}>::unpackCall_{1}(void *o, int argc, void **argv, void *ret) {{"
                    "\t {0} *p = reinterpret_cast<{0}*>(o);",
                    method->getParent()->getQualifiedNameAsString(), identifier);

        out << "void metal::InvokableDispatcher<" << method->getParent()->getQualifiedNameAsString() << ">::unpackCall_" << identifier
            << "(void *o, int argc, void **argv, void *ret) {" << std::endl;
        out << "\t" << method->getParent()->getQualifiedNameAsString() << " *p = reinterpret_cast<" << method->getParent()->getQualifiedNameAsString() << "*>(o);" << std::endl;
        const unsigned argc(method->getNumParams());
        const std::string retTyIdent(genIdentifier(method->getReturnType().getTypePtr()));

        if (!method->getReturnType()->isVoidType()) {
            out << "\t" << method->getReturnType().getAsString(printingPolicy) << " &r = *reinterpret_cast<" << method->getReturnType().getAsString(printingPolicy) << "*>(ret);" << std::endl;
            out << "\tr = p->" << method->getQualifiedNameAsString() << "(";
        }
        else {
            out << "\tp->" << method->getQualifiedNameAsString() << "(";
        }
        if (argc) out << std::endl;
        for (unsigned idx(0); idx < argc; ++idx) {
            auto parm = method->getParamDecl(idx);
            if(!parm->getType()->isReferenceType())
                out << "\t\t*reinterpret_cast<" << parm->getType().getAsString(printingPolicy) << "*>(argv[" << idx << "])";
            else {
                const clang::ReferenceType *rTyp = static_cast<const clang::ReferenceType*>(parm->getType().getTypePtr());
                if (rTyp->isRValueReferenceType()) {
                    out << "\t\tstd::move(";
                }
                else {
                    out << "\t\t";
                }
                out << "*reinterpret_cast<" << rTyp->getPointeeType().getAsString(printingPolicy) << "*>(argv[" << idx << "])";
                if (rTyp->isRValueReferenceType()) {
                    out << ")";
                }

            }

            const std::string argTyIdent(genIdentifier(parm->getType().getTypePtr()));
            if (idx < argc - 1) {
                out << ", ";
            }
            out << std::endl;
        }
        if (argc) out << "\t";
        out << ");" << std::endl;
        out << "}" << std::endl << std::endl << std::endl;

    }

    void RuntimeReflectionDataSourceGenerator::genMethodArgDescriptorList(const clang::CXXMethodDecl *method) {
        const unsigned argc(method->getNumParams());
        if (!argc) return;
        auto methodIdent = genIdentifier(static_cast<const clang::Decl*>(method));

        out << "const metal::Argument " << methodIdent << "_argumentArray[] = { " << std::endl;
        for (unsigned idx(0); idx < argc; ++idx) {
            auto param = method->getParamDecl(idx);
            out << "\t {\"" << param->getNameAsString() << "\", metal::QualType(";
            
            auto argt(param->getType());
            auto argtIdent(genIdentifier(argt.getTypePtr()));
            out << (argt.isConstQualified() ? "true" : "false") << ", ";
            out << (argt.isVolatileQualified() ? "true" : "false") << ", ";
            out << (argt.isRestrictQualified() ? "true" : "false") << ", ";
            out << "&"  << argtIdent<< ")}";
            if (idx < argc - 1) out << ",";
            out << std::endl;
        }
        out << "}; " << std::endl;
        out << std::endl << std::endl;

    }

    void RuntimeReflectionDataSourceGenerator::genMethodDescriptor(const clang::CXXMethodDecl *method) {
        auto identifier(genIdentifier(static_cast<const clang::Decl*>(method)));
        if (isDefined(identifier)) return;

        genMethodArgDescriptorList(method);
        const unsigned argc(method->getNumParams());
        out << "const metal::Method " << identifier << " = {" << std::endl;
        out << "\t"; out << "\"" << method->getNameAsString() << "\"," << std::endl;
        out << "\t"; out << "\"" << method->getQualifiedNameAsString() << "\", " << std::endl;
        out << "\t"; out << argc << ", " << std::endl;
        out << "\t";
        if (argc)
            out << identifier << "_argumentArray, " << std::endl;
        else
            out << "nullptr, " << std::endl;
        out << "\t"; out << (method->isConst() ? 1 : 0) << ", " << std::endl;
        out << "\t"; out << (method->isVirtual() ? 1 : 0) << ", " << std::endl;
        out << "\t&metal::InvokableDispatcher<"  << method->getParent()->getQualifiedNameAsString() << ">::unpackCall_" << identifier << std::endl;
        out << "};" << std::endl;

        out << std::endl << std::endl;
        defineIdentifier(identifier);
    }

    void RuntimeReflectionDataSourceGenerator::genRecordDescriptor(const clang::CXXRecordDecl *record, const std::vector<const clang::CXXMethodDecl*> &exportedMethods) {
        auto identifier(genIdentifier(static_cast<const clang::Decl*>(record)));
        if (isDefined(identifier)) return;

        genRecordMethodDescriptorList(record, exportedMethods);
        out << "const metal::Record " << identifier << " = {" << std::endl;
        out << "\t\"" << record->getQualifiedNameAsString() << "\", " << std::endl;
        out << "\t\"" << record->getNameAsString() << "\", " << std::endl;
        out << "\t" << exportedMethods.size() << ", " << std::endl;
        if (exportedMethods.empty())
            out << "\tnullptr, " << std::endl;
        else {
            out << "\t" << identifier << "_MethodArray, " << std::endl;
        }
            
        out << "};" << std::endl;
        out << std::endl << std::endl;

        defineIdentifier(identifier);
    }

    void RuntimeReflectionDataSourceGenerator::genRecordMethodDescriptorList(const clang::CXXRecordDecl *record, const std::vector<const clang::CXXMethodDecl*> &exportedMethods) {
        if (exportedMethods.empty()) return;
        auto identifier(genIdentifier(static_cast<const clang::Decl*>(record)));
        
        out << "const metal::Method *" << identifier << "_MethodArray[] = {" << std::endl;
        for (unsigned idx(0); idx < exportedMethods.size(); ++idx ) {
            auto &m = exportedMethods[idx];
            auto mIdent(genIdentifier(static_cast<const clang::Decl*>(m)));
            out << "\t&" << mIdent;
            if (idx < exportedMethods.size() - 1) out << ", ";
            out << std::endl;
        }

        out << "};" << std::endl;
    }

    void RuntimeReflectionDataSourceGenerator::genRecordDescBindingToVirtualFcn(const clang::CXXRecordDecl *record) {
        llvm::SmallVector<char, 1024> res;
        llvm::raw_svector_ostream o(res);
        auto identifier(genIdentifier(static_cast<const clang::Decl*>(record)));
        o << "const metal::Record *" << record->getQualifiedNameAsString() << "::metaClass() {\n";
        o << "\treturn &" << identifier << ";\n";
        o << "}\n\n\n";
        out << o.str().str();
    }
    */
}
