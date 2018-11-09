#ifndef Clang_Infrastructure_h_Included
#define Clang_Infrastructure_h_Included

#pragma warning(push, 0)
#include <clang/Tooling/Tooling.h>
#include <clang/AST/ASTConsumer.h>
#pragma warning(pop)

#include "GeneratingASTVisitor.h"
#include "MetaContext.h"
#include <set>
#include <string>
#include <memory>

namespace mc {
    class CodeGeneratorBase;

    class ActionFactory : public clang::tooling::FrontendActionFactory {
    public:
        clang::FrontendAction *create() override;
        ActionFactory();
        ~ActionFactory();

    private:
        mc::Context mcContext;
    };

    class MetadataGenerateAction : public clang::ASTFrontendAction {
    public:
        MetadataGenerateAction(mc::Context &mcContext)
            :mcContext(mcContext),
            compiler(nullptr) {

        }

        virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile);
        virtual bool BeginInvocation(clang::CompilerInstance &CI);
        virtual void EndSourceFileAction();
        // virtual bool BeginSourceFileAction();

    private:
        mc::Context &mcContext;
        clang::CompilerInstance *compiler;
    };

    class MetadataTransformingConsumer : public clang::ASTConsumer {
    public:
        MetadataTransformingConsumer(llvm::StringRef iFile, mc::Context &mcContext, const clang::PrintingPolicy &pPolicy);
        virtual void HandleTranslationUnit(clang::ASTContext &Context);

    private:
        GeneratingASTVisitor visitor;
        std::string inputFile;
        std::vector<std::shared_ptr<mc::CodeGeneratorBase>> generators;

        std::string outputHeader;
        std::string outputSource;
        std::string outputPreamble;
        mc::Context &mcContext;
    };
}

#endif // Clang_Infrastructure_h_Included
