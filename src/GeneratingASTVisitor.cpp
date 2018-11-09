#include "GeneratingASTVisitor.h"

#include <unordered_map>
#include <string_view>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <experimental/filesystem>

#pragma warning(push, 0)
#include <llvm/Support/CommandLine.h>
#pragma warning(pop)


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

    GeneratingASTVisitor::GeneratingASTVisitor(const std::string& outputFile, const clang::PrintingPolicy &pPolicy)
        :out(fs::path(outputFile).replace_extension("metagen.cpp")),
        idman(pPolicy, fs::path(outputFile).replace_extension("idents.json")),
        printingPolicy(pPolicy) {

        out << "#include <MetaType.h>\n";
    }

    GeneratingASTVisitor::~GeneratingASTVisitor() {}

    
    bool GeneratingASTVisitor::VisitCXXRecordDecl(const clang::CXXRecordDecl *record) {
        const bool inMainFile(record->getASTContext().getSourceManager().isInMainFile(record->getLocation()));

        if (inMainFile && record->isThisDeclarationADefinition()) {
            for(const auto method: record->methods()) {
                if (method->getAccess() == clang::AccessSpecifier::AS_public) {
                    genTypeDescriptor(method->getReturnType().getTypePtr());
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

        if (idman.isDefined(identifier)) return;

        switch (ty->getTypeClass()) {
        case clang::Type::TypeClass::Builtin: {
            // auto btIn = static_cast<const clang::BuiltinType*>(ty);
            // const auto mcBuiltInName = builtinTypes.at(btIn->getKind());
            // fmt::print(out, "const metal::BuiltinType {}(metal::BuiltinType::BuiltinKind::{}, \"{}\");",
            //            identifier, mcBuiltInName, btIn->getCanonicalTypeInternal().getAsString(printingPolicy));
            fmt::print(out, "const extern metal::BuiltinType {};\n",
                       identifier);
            idman.expectExternalIdentifier(identifier);
        } break;
        /*
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
            const bool isInMainFile(decl->getASTContext().getSourceManager().isInMainFile(decl->getLocation()));
            if (isInMainFile) {

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
                fmt::u

            }

        } break;
*/
        case clang::Type::TypeClass::Record: {
            auto tty = static_cast<const clang::RecordType*>(ty);

            const bool isInMainFile(tty->getDecl()->getASTContext().getSourceManager().isInMainFile(tty->getDecl()->getLocation()));
            if (isInMainFile) {
                fmt::print(out, "const metal::RecordType {} ( {} {});\n",
                           identifier, idman.id(static_cast<const clang::Decl *>(tty->getAsCXXRecordDecl())), tty->getDecl()->getQualifiedNameAsString());
            } else {
                idman.expectExternalIdentifier(identifier);
                fmt::print(out, "extern const metal::RecordType {};\n", identifier);

            }
        } break;
        default:
            break;
        }
        idman.defineIdentifier(identifier);

    }

}
