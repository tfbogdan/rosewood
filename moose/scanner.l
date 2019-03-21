%{
/* C++ string header, for string ops below */
#include <string>

#include <parser.hpp>
#include <Lexer.h>

#undef  YY_DECL
#define YY_DECL int moose::Lexer::yylex( moose::Parser::semantic_type * const lval, moose::Parser::location_type *location )

/* using "token" to make the returns for the tokens shorter to type */
using token = moose::Parser::token;

/* define yyterminate as this instead of NULL */
#define yyterminate() return token::END

/* msvc2010 requires that we exclude this header file. */
#define YY_NO_UNISTD_H

/* update location on matching */
#define YY_USER_ACTION loc->step(); loc->columns(yyleng);

%}

%option debug
%option nodefault
%option yyclass="moose::Lexer"
%option noyywrap
%option c++


%%
%{  /** Code executed at the beginning of yylex **/
    yylval = lval;
%}

[ \t\r]+ {
}

[\n] {
    loc->lines();
}

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval->build< std::string >(yytext);
    return token::IDENTIFIER;
}

[=] {
    return token::EQ;
}

[(] {
    return token::LPARAN;
}

[)] {
    return token::RPARAN;
}

[,] {
    return token::COMMA;
}

[;] {
    return token::SCOLON;
}

[:] {
    return token::COLON;
}

\"(\\.|[^"\\])*\" {
    yylval->build<std::string>(std::string(yytext + 1, yyleng - 2));
    return token::STRING;
}

%%
