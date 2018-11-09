#pragma warning(push, 0)
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/TargetSelect.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Frontend/CompilerInstance.h>
#pragma warning(pop)

#include <string> 
#include <fmt/printf.h>

#include "clang_infrastructure.h"

llvm::cl::OptionCategory mcOptionsCategory("mc options");
llvm::cl::opt<std::string> mcOutputDirectory("od", llvm::cl::cat(mcOptionsCategory), llvm::cl::Required, llvm::cl::desc("output directory for generated files"));
llvm::cl::opt<std::string> mcIdentifierRepository("idents", llvm::cl::cat(mcOptionsCategory), llvm::cl::Optional, llvm::cl::desc("identifier repository file. if it does not exist, it shall be created. "));

int main(int argc, const char **argv) {

    std::vector<std::string> args;
    fmt::printf("Invocation: ");
    for(unsigned argi(0); argi < argc; ++argi) {
        args.push_back(argv[argi]);
        fmt::printf("%s ", argv[argi]);
    }
	fmt::printf("\n");

    LLVMInitializeX86TargetInfo();
    LLVMInitializeX86TargetMC();
    LLVMInitializeX86AsmParser();

    args.push_back("-xc++");
    // args.push_back("-std=c++17");
    // args.push_back("-E");
    // args.push_back("-v");
    // args.push_back("-");
    // args.push_back("--gcc-toolchain");

    std::vector<const char *> argvv;
    for(unsigned argi(0); argi < argc; ++argi) {
        // argvv.push_back(argv[argi]);
    }
    for(const auto &a: args) {
        argvv.push_back(a.c_str());
    }
    int argcc(argvv.size());

    clang::tooling::CommonOptionsParser OptionsParser(argcc, argvv.data(), mcOptionsCategory);
    clang::tooling::ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

    auto fact = new mc::ActionFactory();
    auto res = Tool.run(fact);
    return res;
}
