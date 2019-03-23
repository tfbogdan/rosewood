#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <rosewood/runtime.hpp>

namespace moose::ast {

    // struct Decl {
    //     virtual ~Decl() = 0;
    // };

    struct Stmt {
        virtual ~Stmt() = 0;
    };

    struct TypeRef {
        const rosewood::TypeDeclaration* td;
    };

    struct ValueStmt : public Stmt {};

    struct Expr : public ValueStmt {
        virtual bool typecheck() = 0;
    };

    struct StringLiteral : public Expr {
        StringLiteral(std::string_view sv)
            :value(sv) {}

        bool typecheck() final { return true; }

        std::string value;
    };

    struct IntegralLiteral : public Expr {
        IntegralLiteral(std::int64_t v)
            :value(v) {}

        bool typecheck() final { return true; }

        std::int64_t value;
    };

    struct FloatingPLiteral : public Expr {
        FloatingPLiteral(double v)
            :value(v) {}

        bool typecheck() final { return true; }

        double value;
    };

    struct BinaryOperator : public Expr {
        BinaryOperator(std::shared_ptr<Expr> lhs, std::string_view op, std::shared_ptr<Expr> rhs)
            : op(op),
              lhs(lhs),
              rhs(rhs) {}

        bool typecheck() final { return false; } // 1: determine type of lhs; 2: is rhs addable to it?

        std::string op;
        std::shared_ptr<Expr> lhs;
        std::shared_ptr<Expr> rhs;
    };

    struct CallExpr : public Expr {

        bool typecheck() final { return false; }
        // TDOODODO
    };

    struct VarDecl : public Expr {
        template <typename iter_t>
        VarDecl(std::string_view nm, TypeRef tp, iter_t fparam, iter_t lparam)
            : name(nm),
              type(tp.td),
              params(fparam, lparam) {

        }

        bool typecheck() final;
        std::string name;
        const rosewood::TypeDeclaration *type;
        std::vector<std::shared_ptr<Expr>> params;
    };

}
