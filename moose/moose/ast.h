#pragma once

#include <string>
#include <cstdint>
#include <memory>

namespace moose::ast {

    // struct Decl {
    //     virtual ~Decl() = 0;
    // };

    struct Stmt {
        virtual ~Stmt() = 0;
    };

    struct ValueStmt : public Stmt {};

    struct Expr : public ValueStmt {};

    struct StringLiteral : public Expr {
        StringLiteral(std::string_view sv)
            :value(sv) {}

        std::string value;
    };

    struct IntegralLiteral : public Expr {
        IntegralLiteral(std::int64_t v)
            :value(v) {}

        std::int64_t value;
    };

    struct FloatingPLiteral : public Expr {
        FloatingPLiteral(double v)
            :value(v) {}

        double value;
    };

    struct BinaryOperator : public Expr {
        BinaryOperator(std::shared_ptr<Expr> lhs, std::string_view op, std::shared_ptr<Expr> rhs)
            : op(op),
              lhs(lhs),
              rhs(rhs) {}

        std::string op;
        std::shared_ptr<Expr> lhs;
        std::shared_ptr<Expr> rhs;
    };

    struct CallExpr : public Expr {

        // TDOODODO
    };

    struct ParameterList {

    };
}
