#include "ClangInfrastructure.h"

#pragma warning(push, 0)
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/PreprocessorOptions.h>
#include <llvm/Support/raw_ostream.h>
#include <experimental/filesystem>

#include <llvm/Support/Path.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <clang/AST/AST.h>

#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/FrontendOptions.h>
#include <clang/Sema/Sema.h>
#include <clang/Parse/ParseAST.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/ASTConsumers.h>
#include <clang/Lex/HeaderSearch.h>
#pragma warning(pop)

namespace mc {

    clang::FrontendAction *ActionFactory::create() {
        return new MetadataGenerateAction;
    }

    std::unique_ptr<clang::ASTConsumer> MetadataGenerateAction::CreateASTConsumer(clang::CompilerInstance &Compiler, [[maybe_unused]] llvm::StringRef InFile) {
        compiler = &Compiler;
        return std::make_unique<MetadataTransformingConsumer>(Compiler.getASTContext());
    }

    bool MetadataGenerateAction::BeginInvocation(clang::CompilerInstance &CI) {        
        return ASTFrontendAction::BeginInvocation(CI);
    }

    MetadataTransformingConsumer::MetadataTransformingConsumer(const clang::ASTContext &context)
        :generator(context) {}

    void MetadataTransformingConsumer::HandleTranslationUnit(clang::ASTContext &) {
        generator.Generate();
    }

}
