#include "driver.h"
#include <iostream>

namespace moose {

    void Driver::typecheck(moose::ast::Stmt* stmt) {

    }

    Driver::IdentType Driver::identifierType(std::string_view ident) {
        auto res = index.getDeclaration(ident);
        if (res) {
            if (res->asEnum() || res->asClass()) {
                return IdentType::Type;
            }

        }
        return IdentType::Undefined;
    }

    const rosewood::Declaration *Driver::getDeclaration(std::string_view ident) {
        return index.getDeclaration(ident);
    }

    void Driver::pushExpression(std::shared_ptr<moose::ast::Expr> expr) {
        std::cerr << "About now I should start evaluating expressions.\n";
    }
}
