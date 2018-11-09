#include "clang_infrastructure.h"

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

#include "StaticReflectionDataHeaderGenerator.h"
#include "RuntimeReflectionDataSourceGenerator.h"
#include "MetaPreambleHeaderGenerator.h"

namespace mc {
    ActionFactory::ActionFactory() {
    }

    ActionFactory::~ActionFactory() {
    }

    clang::FrontendAction *ActionFactory::create() {
        return new MetadataGenerateAction(mcContext);
    }

    std::unique_ptr<clang::ASTConsumer> MetadataGenerateAction::CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
        compiler = &Compiler;
        /*Compiler.getPreprocessor().getHeaderSearchInfo().getHeaderSearchOpts().UseBuiltinIncludes = true;
        Compiler.getPreprocessor().getHeaderSearchInfo().getHeaderSearchOpts().UseStandardCXXIncludes = true;
        Compiler.getPreprocessor().getHeaderSearchInfo().getHeaderSearchOpts().UseStandardSystemIncludes = true;
        Compiler.getPreprocessor().getHeaderSearchInfo().getHeaderSearchOpts().Verbose = true;
        */
        return std::make_unique<MetadataTransformingConsumer>(InFile, mcContext, Compiler.getASTContext().getPrintingPolicy());
    }

    bool MetadataGenerateAction::BeginInvocation(clang::CompilerInstance &CI) {
        CI.getLangOpts().CPlusPlus = 1;
        CI.getLangOpts().CPlusPlus11 = 1;
        CI.getLangOpts().CPlusPlus14 = 1;
        CI.getLangOpts().CPlusPlus17 = 1;
        CI.getLangOpts().Bool = 1;

        // CI.getFrontendOpts().ShowStats = 1;
        CI.getHeaderSearchOpts().UseBuiltinIncludes = true;
		CI.getHeaderSearchOpts().UseStandardCXXIncludes = true;
		CI.getHeaderSearchOpts().UseStandardSystemIncludes = true;
		// CI.getHeaderSearchOpts().UseLibcxx = false;
        // CI.getHeaderSearchOpts().Verbose = true;
        
        CI.getPreprocessorOpts().addMacroDef("__MC__PARSER_RUNNING");
        
        return ASTFrontendAction::BeginInvocation(CI);
    }

    void MetadataGenerateAction::EndSourceFileAction() {
        /** Do not remove this comment! It serves as a good example of how we could go change an existing AST
            This might come in useful later if we decide to switch to AST node generation
            instead of textual code generation. 

            For now, we go with textual generation as it looks like an AST based approach has a high upfront cost
            and its benefits are not immediately apparent. 

        if (compiler) {
            
            clang::CompilerInstance newCompiler;
            newCompiler.getLangOpts() = compiler->getLangOpts();
            newCompiler.getFrontendOpts() = compiler->getFrontendOpts();
            newCompiler.getTargetOpts() = compiler->getTargetOpts();
            newCompiler.getCodeGenOpts() = compiler->getCodeGenOpts();
            newCompiler.getDiagnosticOpts() = compiler->getDiagnosticOpts();
            newCompiler.getFileSystemOpts() = compiler->getFileSystemOpts();
            newCompiler.getPreprocessorOpts() = compiler->getPreprocessorOpts();
            newCompiler.getPreprocessorOutputOpts() = compiler->getPreprocessorOutputOpts();
            newCompiler.getHeaderSearchOpts() = compiler->getHeaderSearchOpts();
            newCompiler.getAnalyzerOpts() = compiler->getAnalyzerOpts();

            newCompiler.setTarget(&compiler->getTarget());
            newCompiler.createDiagnostics();
            newCompiler.createFileManager();
            newCompiler.createSourceManager(newCompiler.getFileManager());
            newCompiler.createPreprocessor(clang::TranslationUnitKind::TU_Complete);
            newCompiler.createASTContext();


            // newCompiler.setASTConsumer(std::make_unique<clang::ASTConsumer>());
            newCompiler.setASTConsumer(clang::CreateASTDumper("", true, false, false));

            

            newCompiler.createSema(clang::TranslationUnitKind::TU_Complete, nullptr);

            newCompiler.getASTContext().createMangleContext();
            newCompiler.getASTContext().createMangleNumberingContext();
            
            newCompiler.getPreprocessor().createPreprocessingRecord();
           
            // const clang::FileEntry *e = newCompiler.getFileManager().getFile("c:\\work\\metropolis_build\\Metropolis\\Vulkan.metagen.h");
            
            // auto fid = newCompiler.getSourceManager().getOrCreateFileID(e, clang::SrcMgr::CharacteristicKind::C_User);
            // newCompiler.getSourceManager().setMainFileID(fid);
            // clang::CreateASTViewer();
            newCompiler.getDiagnosticClient().BeginSourceFile(
                newCompiler.getLangOpts(),
                &newCompiler.getPreprocessor());

            clang::SyntaxOnlyAction act;
            clang::InputKind ik(clang::InputKind::Language::CXX, clang::InputKind::Format::Source, true);
            std::error_code ec;
            llvm::raw_fd_ostream osd(llvm::StringRef(std::string("c:\\work\\metropolis_build\\Metropolis\\Vulkan.metagen.h")), ec, llvm::sys::fs::OpenFlags::F_Text);
            osd << "#include <MetaType.h>\n";
            osd << "extern metal::QualType sometyp;\n";
            osd << "metal::QualType sometyp = metal::QualType(false, false, false, nullptr);\n";
            osd << "metal::QualType constructingTyp(false, false, false, nullptr);\n";

            osd << "\n\n";
            osd.close();

            clang::FrontendInputFile ifile("c:\\work\\metropolis_build\\Metropolis\\Vulkan.metagen.h", ik);
            newCompiler.InitializeSourceManager(ifile);
            // newCompiler.ExecuteAction(act);
            
            llvm::outs() << "start parsing the ast\n";

            clang::ParseAST(newCompiler.getPreprocessor(), &newCompiler.getASTConsumer(), newCompiler.getASTContext());
            llvm::outs() << "done parsing the ast\n";
            llvm::outs().flush();

            llvm::outs() << "metal collector at work\n";
            llvm::outs().flush();

            MetalTypesCollector mcCollector;
            mcCollector.TraverseTranslationUnitDecl(newCompiler.getASTContext().getTranslationUnitDecl());

            clang::ASTContext   &aContext = newCompiler.getASTContext();
            clang::DeclContext  *tu = aContext.getTranslationUnitDecl();
            clang::IdentifierTable &idt = aContext.Idents;
            clang::Sema &sema = newCompiler.getSema();
            // clang::BinaryOperator *assignment = new (aContext) clang::BinaryOperator(nullptr, nullptr, clang::BinaryOperatorKind::BO_Assign, clang::QualType(), clang::ExprValueKind::VK_RValue, clang::ExprObjectKind::OK_Ordinary, clang::SourceLocation(), clang::FPOptions()) ;
            clang::IdentifierInfo * iInfo = &idt.get("anotherVariable");
            clang::QualType idtyp (mcCollector.metalQualType.recordDecl->getTypeForDecl(), 0);
            
            llvm::outs() << "creating VarDecl\n";
            llvm::outs().flush();

            clang::VarDecl *newDec = clang::VarDecl::Create(aContext, tu, clang::SourceLocation(), clang::SourceLocation(), iInfo, idtyp, aContext.CreateTypeSourceInfo(idtyp), clang::StorageClass::SC_None);
            newDec->setInitStyle(clang::VarDecl::InitializationStyle::CallInit);
            clang::ASTContext::GetBuiltinTypeError err;
            clang::QualType nullptr_t_t;
            for (auto &t : aContext.getTypes()) {
                if (t->getTypeClass() == clang::Type::Builtin) {
                    clang::BuiltinType *tt = static_cast<clang::BuiltinType*>(t);
                    if (tt->getKind() == clang::BuiltinType::Kind::NullPtr) {
                        nullptr_t_t = clang::QualType(tt, 0);
                        break;
                    }
                }
            }
            clang::CXXBoolLiteralExpr *arg1 = new (aContext) clang::CXXBoolLiteralExpr(false, mcCollector.metalQualType.ctor->getParamDecl(0)->getType(), clang::SourceLocation());
            clang::CXXBoolLiteralExpr *arg2 = new (aContext) clang::CXXBoolLiteralExpr(false, mcCollector.metalQualType.ctor->getParamDecl(1)->getType(), clang::SourceLocation());
            clang::CXXBoolLiteralExpr *arg3 = new (aContext) clang::CXXBoolLiteralExpr(false, mcCollector.metalQualType.ctor->getParamDecl(2)->getType(), clang::SourceLocation());
            clang::CXXNullPtrLiteralExpr *nullPtrExpr = new (aContext) clang::CXXNullPtrLiteralExpr(nullptr_t_t, clang::SourceLocation());
            clang::ImplicitCastExpr *arg4 = clang::ImplicitCastExpr::Create(aContext, mcCollector.metalQualType.ctor->getParamDecl(3)->getType(), clang::CastKind::CK_NullToPointer, nullPtrExpr, nullptr, clang::ExprValueKind::VK_RValue);

            clang::Expr *aargs[] = { arg1, arg2, arg3, arg4 };
            llvm::ArrayRef<clang::Expr*> args(aargs);
            llvm::outs() << "creating construct expr\n";
            llvm::outs().flush();

            clang::CXXConstructExpr *ctorExpr = clang::CXXConstructExpr::Create(
                aContext, idtyp, clang::SourceLocation(), 
                mcCollector.metalQualType.ctor, 
                false, 
                args, 
                false, false, false, false, clang::CXXConstructExpr::ConstructionKind::CK_Complete, clang::SourceRange()
            );
            llvm::outs() << "adding init expr to ctr expr\n";
            llvm::outs().flush();

            newDec->setInit(ctorExpr);
            llvm::outs() << "ctor expr to context\n";
            llvm::outs().flush();


            aContext.getTranslationUnitDecl()->addDecl(newDec);
                // newCompiler.getASTConsumer().HandleTranslationUnit(newCompiler.getASTContext());
                // newCompiler.getASTContext()
            llvm::outs() << "trying to print again\n";
            llvm::outs().flush();


            // newCompiler.getASTConsumer().HandleTranslationUnit(newCompiler.getASTContext());
            // std::string output;
            llvm::SmallVector<char, 254545> output;

            // new// Compiler.setASTConsumer(clang::CreateASTPrinter(std::make_unique<llvm::raw_string_ostream>(output), ""));
            auto pprinter = clang::CreateASTPrinter(std::make_unique<llvm::raw_svector_ostream>(output), "");
            pprinter->HandleTranslationUnit(aContext);
            for (volatile unsigned idx(0); idx < 100000; ++idx) {
                ;
            }
            llvm::outs() << output;
        }
        */
    }

    MetadataTransformingConsumer::MetadataTransformingConsumer(llvm::StringRef iFile, mc::Context &mcContext, const clang::PrintingPolicy &pPolicy)
        :visitor(pPolicy),
        inputFile(iFile),
        mcContext(mcContext) {}

    void MetadataTransformingConsumer::HandleTranslationUnit(clang::ASTContext &Context) {
        visitor.TraverseTranslationUnitDecl(Context.getTranslationUnitDecl());
    }

}
