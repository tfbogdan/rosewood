#include <moose/ast.h>

namespace moose::ast {

    Stmt::~Stmt() = default;


    bool VarDecl::typecheck() {
        return false;
    }
}
