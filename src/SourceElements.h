#ifndef SourceElements_h_Included
#define SourceElements_h_Included

#include <string>
#include <memory>

#pragma warning(push, 0)
#include <llvm/Support/raw_ostream.h>
#include <llvm/ADT/SmallVector.h>
#include <clang/AST/Type.h>
#pragma warning(pop)

namespace clang {

}

namespace mc {

    struct IncludeDirective {
        IncludeDirective(const std::string &f, bool local)
            :file(f),
            is_local(local) {}

        std::string     file;
        bool is_local = false;

        void toCode(llvm::raw_ostream &o) const;
    };

    struct Expression {

        virtual ~Expression() = default;
        virtual void toCode(llvm::raw_ostream &o) const = 0;


    };

    struct AnonymousVariable : public Expression {

        AnonymousVariable(const std::string &typeName, const std::vector<std::shared_ptr<Expression>> &inits)
            :type(typeName),
            initializers(inits) {}

        std::string type;
        std::vector<std::shared_ptr<Expression>> initializers;

        virtual ~AnonymousVariable() = default;
        virtual void toCode(llvm::raw_ostream &o) const;
    };

    struct LiteralExpression : public Expression {
        LiteralExpression(const std::string &expr)
            :expression(expr) {}

        std::string expression;

        virtual ~LiteralExpression() = default;
        virtual void toCode(llvm::raw_ostream &o) const ;
    };

    struct CStringLiteral : public Expression {
        CStringLiteral(const std::string &val)
            :value(val) {}

        std::string value;
        virtual ~CStringLiteral() = default;
        virtual void toCode(llvm::raw_ostream &o) const;

    };

    struct BoolLiteral : public Expression {
        BoolLiteral(bool v)
            :value(v) {}

        bool value;

        virtual ~BoolLiteral() = default;
        virtual void toCode(llvm::raw_ostream &o) const;
    };

    struct AddressOf : public Expression {
        AddressOf(const std::string &of)
            :ident(of) {}

        std::string ident;

        virtual ~AddressOf() = default;
        virtual void toCode(llvm::raw_ostream &o) const;
    };

    struct VariableDeclaration : public Expression {
        VariableDeclaration(const std::string &typeName, const std::string &ident)
            :type(typeName),
            identifier(ident) {}

        std::string     type;
        std::string     identifier;

        virtual ~VariableDeclaration() = default;
        virtual void toCode(llvm::raw_ostream &os) const;
    };

    struct VariableDefinition : public Expression {
        VariableDefinition(const std::string &typeName, const std::string &ident, const std::vector<std::shared_ptr<Expression>> &init)
            :type(typeName),
            identifier(ident),
            initializers(init.begin(), init.end()) {}

        std::string     type;
        std::string     identifier;
        llvm::SmallVector<std::shared_ptr<Expression>, 24> initializers;

        virtual void toCode(llvm::raw_ostream &os) const;
    };

}



#endif // SourceElements_h_Included
