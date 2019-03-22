%require  "3.0"
%language "C++"
%debug
%defines
%define api.namespace {moose}
%define parser_class_name {Parser}
%define parse.error verbose

%code requires {
    namespace moose {
        class Lexer;
        class Driver;
    }
#include <memory>

#include <moose/ast.h>

#undef YY_NULLPTR
#define YY_NULLPTR nullptr

#include <string_view>
}

%parse-param {Lexer &lexer}
%parse-param {Driver &driver}


%code{
#include <iostream>
#include <string>
#include <Lexer.h>

#include <fmt/format.h>

#undef yylex
#define yylex lexer.yylex
}

%define api.value.type variant
%define parse.assert

%token                          END                 0       "end of input"
%token                          BLANK
%token                          NEWLINE
%token <std::string>            IDENTIFIER
%token                          RPARAN
%token                          LPARAN
%token                          COMMA
%token                          SCOLON
%token                          COLON
%token <std::string>            STRING

%token <std::int64_t>           INTEGER_NUM
%token <double>                 FLOATIN_NUM

%token                          EQ
%token                          PLUS
%token                          MINUS
%token                          EQEQ
%token                          ASTERISK


%locations

%type <std::shared_ptr<moose::ast::Expr>>                   literal parameter expression
%type <std::shared_ptr<moose::ast::BinaryOperator>>         binary_op
%type <std::shared_ptr<moose::ast::CallExpr>>               call_expr
%%
script:
        block END
    ;

block:
        block statement
    |   statement
    ;

statement:
        expression SCOLON                   {}
    ;

expression:
        call_expr                           {$$ = $1;}
    |   literal                             {$$ = $1;}
    |   binary_op                           {$$ = $1;}
    ;

binary_op:
        expression PLUS expression          { $$ = std::make_shared<moose::ast::BinaryOperator>($1, "+", $3);}
    |   expression EQ expression            { $$ = std::make_shared<moose::ast::BinaryOperator>($1, "=", $3);}
    |   expression MINUS expression         { $$ = std::make_shared<moose::ast::BinaryOperator>($1, "-", $3);}
    ;

call_expr:
        IDENTIFIER LPARAN parameter_list RPARAN { $$ = std::make_shared<moose::ast::CallExpr>();}
    ;

parameter_list:
        parameter_list COMMA parameter  {std::cerr << "parameter list \n";}
    |   parameter                       {std::cerr << "parameter list \n";}
    |   %empty                          {std::cerr << "parameter list (empty)\n";}
    ;

parameter:
        IDENTIFIER {
            std::cerr << fmt::format("Puhsing identifier arg {}\n", $1);
        }
    |   expression {
            $$ = $1;
        }
    |   literal {
            $$ = $1;
        }
    ;


literal:
        STRING  {
            $$ = std::make_shared<moose::ast::StringLiteral>($1);
            std::cerr << "Pushing string literal \"" << $1 << "\"\n";
        }
    |   INTEGER_NUM {
            $$ = std::make_shared<moose::ast::IntegralLiteral>($1);
            std::cerr << "Pushing integer literal \"" << $1 << "\"\n";
        }
    |   FLOATIN_NUM {
            $$ = std::make_shared<moose::ast::FloatingPLiteral>($1);
            std::cerr << "Pushing float literal \"" << $1 << "\"\n";
        }
    ;
%%

void moose::Parser::error( const location_type &l, const std::string &err_message )
{
   std::cerr << "Error: " << err_message << " at " << l << "\n";
}
