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
#include <rosewood/index.hpp>

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

%token <moose::ast::TypeRef>                    TYPENAME
%token <std::string>                            UNDEFINED_QUALNAME
%token <std::string>                            NEW_IDENTIFIER


%locations

%type <std::shared_ptr<moose::ast::Expr>>                   literal expression
%type <std::shared_ptr<moose::ast::BinaryOperator>>         binary_op
%type <std::shared_ptr<moose::ast::CallExpr>>               call_expr
%type <std::shared_ptr<moose::ast::VarDecl>>                var_decl
%type <std::vector<std::shared_ptr<moose::ast::Expr>>>      parameter_list
%type <std::vector<std::shared_ptr<moose::ast::Expr>>>      cslist;

%%
script:
        block END
    ;

block:
        block statement
    |   statement
    ;

statement:
        expression SCOLON                   { driver.pushExpression($1);}
    |   SCOLON                              { /*It's a nop*/ }
    |   cslist SCOLON                       { driver.pushExpression($1.begin(), $1.end()); }
    ;

expression:
        call_expr                           {$$ = $1;}
    |   literal                             {$$ = $1;}
    |   binary_op                           {$$ = $1;}
    |   var_decl                            {$$ = $1;}
    // |   cslist                              { /* TDO */ }
    ;

var_decl:
        NEW_IDENTIFIER COLON TYPENAME parameter_list { $$ = std::make_shared<moose::ast::VarDecl>($1, $3, $4.begin(), $4.end()); }
    ;

binary_op:
        expression PLUS expression          { $$ = std::make_shared<moose::ast::BinaryOperator>($1, "+", $3);}
    |   expression EQ expression            { $$ = std::make_shared<moose::ast::BinaryOperator>($1, "=", $3);}
    |   expression MINUS expression         { $$ = std::make_shared<moose::ast::BinaryOperator>($1, "-", $3);}
    ;

call_expr:
        IDENTIFIER parameter_list { $$ = std::make_shared<moose::ast::CallExpr>();}
    ;

parameter_list:
        LPARAN cslist RPARAN    { $$ = std::move($2); }
    |   LPARAN RPARAN           { }
    ;

cslist:
        expression                      { $$.emplace_back($1);}
    |   expression COMMA cslist         { $$.emplace_back($1); $$.insert($$.end(), $3.begin(), $3.end()); }
    ;

literal:
        STRING  {
            $$ = std::make_shared<moose::ast::StringLiteral>($1);
        }
    |   INTEGER_NUM {
            $$ = std::make_shared<moose::ast::IntegralLiteral>($1);
        }
    |   FLOATIN_NUM {
            $$ = std::make_shared<moose::ast::FloatingPLiteral>($1);
        }
    ;
%%

void moose::Parser::error( const location_type &l, const std::string &err_message )
{
   std::cerr << "Error: " << err_message << " at " << l << "\n";
}
