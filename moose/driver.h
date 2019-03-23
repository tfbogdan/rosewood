#pragma once

#include <rosewood/index.hpp>
#include <moose/ast.h>

#include <string_view>

namespace moose {

    class Driver {
    public:

        enum IdentType {
            Type,
            Variable,
            Function,
            Undefined
        };

        Driver(rosewood::Index& i)
            :index(i) {}

        void typecheck(moose::ast::Stmt* stmt);
        IdentType identifierType(std::string_view ident);
        IdentType declType(const rosewood::Declaration *decl);

        const rosewood::Declaration *getDeclaration(std::string_view ident);
        void pushExpression(std::shared_ptr<moose::ast::Expr> expr);
        template<typename iter_t>
        void pushExpression(iter_t first, iter_t last) {
            while(first != last) {
                pushExpression(*first);
                ++first;
            }
        }
    private:
        rosewood::Index &index;
    };

}
