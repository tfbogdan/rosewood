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

        const rosewood::DMetaDecl *getDeclaration(std::string_view ident);
    private:
        rosewood::Index &index;
    };

}
