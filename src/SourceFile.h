#ifndef SourceFile_h_Included
#define SourceFile_h_Included


#pragma warning(push, 0)
#include <clang/AST/AST.h>
#include <clang/AST/Type.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/SmallSet.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/raw_ostream.h>
#pragma warning(pop)

#include <vector>
#include <string>


#include "SourceElements.h"

namespace mc {
        
    std::string replaceIllegalIdentifierChars(std::string_view name);

    class SourceFile {
    public:
        SourceFile() = delete;
        SourceFile(const std::string &name)
            :fileName(name) {}

        void addIncludeGuard() { options.includeGuard = 1; }
        
        // void defineInstance(clang::QualType type, std::string identifierName) {}
        void Include(const std::string &includedFile, bool local = false);

        // void DeclareVariable(const std::string &type, const std::string &name);
        // void DefineVariable(const std::string &type, const std::string &, std::vector<std::string> initializers);

        void compile(llvm::raw_ostream &o);
        void addExpression(std::shared_ptr<Expression> expr);
    private:
        std::string fileName;

        struct {
            unsigned includeGuard : 1;
        } options = { 0 };
        
        llvm::SmallVector<mc::IncludeDirective, 10>                 includes;
        llvm::SmallVector<std::shared_ptr<mc::Expression>, 2048>    expressions;
        std::set<std::string>                                       identifierTable;
    };

}


#endif // SourceFile_h_Included
