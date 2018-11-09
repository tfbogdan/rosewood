#pragma once

#include "CodeGeneratorBase.h"

#pragma warning(push, 0)
#include <clang/AST/RecursiveASTVisitor.h>
#pragma warning(pop)

#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>

namespace mc {

    class GeneratingASTVisitor 
        : public clang::RecursiveASTVisitor<GeneratingASTVisitor> {

    public:
        explicit GeneratingASTVisitor(const clang::PrintingPolicy &pPolicy);
        ~GeneratingASTVisitor();

        bool VisitCXXRecordDecl(const clang::CXXRecordDecl *Record);
        bool VisitEnumDecl(const clang::EnumDecl *Enum);
        bool VisitCXXMethodDecl(const clang::CXXMethodDecl *Method);
        bool VisitFunctionDecl(const clang::FunctionDecl *Function);

        std::vector<const clang::CXXRecordDecl*>    exportedRecords;
        std::vector<const clang::EnumDecl*>         exportedEnums;
        std::vector<const clang::FunctionDecl*>     exportedFunctions;

    private:
        bool exportDeclaration(const clang::Decl *Decl);
        void genTypeDescriptor(const clang::Type *ty);
        void genMethodCallUnpacker(const clang::CXXMethodDecl *method);
        void genMethodArgDescriptorList(const clang::CXXMethodDecl *method);
        void genMethodDescriptor(const clang::CXXMethodDecl *method);
        void genRecordDescriptor(const clang::CXXRecordDecl *record, const std::vector<const clang::CXXMethodDecl*> &exportedMethods);
        void genRecordMethodDescriptorList(const clang::CXXRecordDecl *record, const std::vector<const clang::CXXMethodDecl*> &exportedMethods);
        void genRecordDescBindingToVirtualFcn(const clang::CXXRecordDecl *record);


        std::ofstream out;
        mc::IdentifierHelper idman;
        const clang::PrintingPolicy &printingPolicy;
    };


}
