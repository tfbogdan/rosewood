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
    }

#undef YY_NULLPTR
#define YY_NULLPTR nullptr
}

%parse-param {Lexer &lexer}

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
%token                          EQ
%token                          RPARAN
%token                          LPARAN
%token                          COMMA
%token                          SCOLON
%token                          COLON
%token <std::string>            STRING

%locations

%type <std::string>             literal

%%
script:
        block END
    ;

block:
        block expression
    |   expression
    ;

expression:
        assignment
    |   function_call
    |   literal
    ;

assignment:
        IDENTIFIER EQ expression SCOLON {
            std::cerr << fmt::format("assignment to {}\n", $1);
        }
    ;

function_call:
        IDENTIFIER LPARAN parameter_list RPARAN SCOLON {
            std::cerr << fmt::format("Calling {}\n", $1);
        }
    ;

parameter_list:
        parameter_list COMMA parameter
    |   parameter
    |   %empty
    ;

parameter:
        IDENTIFIER {
            std::cerr << fmt::format("Puhsing identifier arg {}\n", $1);
        }
    |   expression {
            std::cerr << "Pushing expression arg\n";
        }

    |   literal {
            std::cerr << "Pushing literal arg \"" << $1 << "\"\n";
        }
    ;


literal:
        STRING  {
            $$ = $1;
            std::cerr << "Pushing string literal \"" << $1 << "\"\n";
        }
    ;
%%

void moose::Parser::error( const location_type &l, const std::string &err_message )
{
   std::cerr << "Error: " << err_message << " at " << l << "\n";
}
