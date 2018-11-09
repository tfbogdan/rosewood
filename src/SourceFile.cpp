#include "SourceFile.h"
#include <initializer_list>

namespace mc {

    const std::map<std::string, std::string> specialCharacterMap{
        { "*", "_ptr_" },
        { "&", "_lvRef_" },
        { "&&","_rvRef_" },
        { "<", "_lt_" },
        { ">", "_gt_" },
        { "::", "_sr_" },
        { " ", "_" },
        { ",", "_comma_" },
        { ".", "_dot_"}
    };

    std::string replaceIllegalIdentifierChars(std::string_view name) {
        std::string res(name);
        for (const auto &illegalSeq : specialCharacterMap) {
            auto index = res.find(illegalSeq.first, 0);
            while (index != std::string::npos) {
                res.erase(index, illegalSeq.first.length());
                res.insert(index, illegalSeq.second);
                index = res.find(illegalSeq.first, 0);
            }
        }
        return res;
    }

    void SourceFile::Include(const std::string &includedFile, bool local) {
        includes.emplace_back(includedFile, local);
    }
    /*
    void SourceFile::DeclareVariable(const std::string &type, const std::string &name) {
        varDecls.emplace_back(type, name);
    }

    void SourceFile::DefineVariable(const std::string &type, const std::string &name, std::vector<std::string> initializers) {
        std::vector<mc::Initializer> inits(initializers.begin(), initializers.end());
        varDefs.emplace_back(type, name, inits);
        DeclareVariable(type, name);
    }
    */
    void SourceFile::addExpression(std::shared_ptr<Expression> expr) {
        expressions.push_back(expr);
    }

    void SourceFile::compile(llvm::raw_ostream &o) {
        llvm::SmallString<128> includeGuard;
        if (options.includeGuard) {
            includeGuard = replaceIllegalIdentifierChars(fileName);

            o << "#ifndef " << includeGuard << "\n";
            o << "#define " << includeGuard << "\n";

        }
        // 
        for (const auto &includedFile : includes) {
            includedFile.toCode(o);
        }
        o << "\n\n";

        for (const auto &varDecl : expressions) {
            varDecl->toCode(o);
            o << "\n\n";

        }

        // 
        if (options.includeGuard) {
            o << "#endif //" << includeGuard << "\n";
        }
    }
}
