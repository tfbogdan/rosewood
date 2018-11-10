#ifndef Clang_Infrastructure_h_Included
#define Clang_Infrastructure_h_Included

#pragma warning(push, 0)
#include <clang/Tooling/Tooling.h>
#include <clang/AST/ASTConsumer.h>
#pragma warning(pop)

#include "ReflectionDataGenerator.h"
#include <set>
#include <string>
#include <memory>


extern llvm::cl::OptionCategory mcOptionsCategory;
extern llvm::cl::opt<std::string> mcOutput;
extern llvm::cl::opt<std::string> mcModuleName;
extern llvm::cl::opt<std::string> mcJsonOutput;

namespace mc {

    class CodeGeneratorBase;

    class ActionFactory : public clang::tooling::FrontendActionFactory {
    public:
        clang::FrontendAction *create() override;
    };

    class MetadataGenerateAction : public clang::ASTFrontendAction {
    public:
        virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile);
        virtual bool BeginInvocation(clang::CompilerInstance &CI);
    private:
        clang::CompilerInstance *compiler = nullptr;
    };

    class MetadataTransformingConsumer : public clang::ASTConsumer {
    public:
        MetadataTransformingConsumer(const clang::ASTContext &context);
        virtual void HandleTranslationUnit(clang::ASTContext &Context);
    private:
        ReflectionDataGenerator generator;
    };
}

#endif // Clang_Infrastructure_h_Included
