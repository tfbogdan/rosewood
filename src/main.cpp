#pragma warning(push, 0)
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/TargetSelect.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Frontend/CompilerInstance.h>
#pragma warning(pop)

#include <string> 
#include <fmt/printf.h>

#include "ClangInfrastructure.h"

llvm::cl::OptionCategory mcOptionsCategory("mc options");
llvm::cl::opt<std::string> mcOutput("o", llvm::cl::cat(mcOptionsCategory), llvm::cl::Required, llvm::cl::desc("cpp metadata output file"));
llvm::cl::opt<std::string> mcModuleName("n", llvm::cl::cat(mcOptionsCategory), llvm::cl::Required, llvm::cl::desc("module name"));
llvm::cl::opt<std::string> mcJsonOutput("j", llvm::cl::cat(mcOptionsCategory), llvm::cl::Required, llvm::cl::desc("json metadata output file"));

// useful for debugging
void printInvokation(int argc, const char **argv) {
    fmt::print("Invocation: ");
    for(int idx(0); idx < argc; ++idx) {
        fmt::print("{} ", argv[idx]);
    }
    fmt::print("\n");
}

int main(int argc, const char **argv) {
    printInvokation(argc, argv);

    clang::tooling::CommonOptionsParser OptionsParser(argc, argv, mcOptionsCategory);
    // right now json file generation is not implemented but the interface exists so just generate a dummy file
    std::ofstream jsonFile(mcJsonOutput);
    jsonFile << "dummy\n";

    clang::tooling::ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

    auto fact = new mc::ActionFactory();
    auto res = Tool.run(fact);
    return res;
}
