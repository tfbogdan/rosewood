%{
/* C++ string header, for string ops below */
#include <string>
#include <fmt/format.h>
#include <parser.hpp>
#include <Lexer.h>
#include <sstream>
#include <moose/ast.h>

#undef  YY_DECL
#define YY_DECL int moose::Lexer::yylex( moose::Parser::semantic_type * const lval, moose::Parser::location_type *location )

/* using "token" to make the returns for the tokens shorter to type */
using token = moose::Parser::token;

/* define yyterminate as this instead of NULL */
#define yyterminate() return token::END

/* msvc2010 requires that we exclude this header file. */
#define YY_NO_UNISTD_H

/* update location on matching */
// #define YY_USER_ACTION loc->step(); loc->columns(yyleng);
#define YY_USER_ACTION location->step(); location->columns(yyleng);

%}

%option debug
%option nodefault
%option yyclass="moose::Lexer"
%option noyywrap
%option c++
%option yylineno


%x COMMENT
%x NESTED_IDENTIFIER

%%
%{  /** Code executed at the beginning of yylex **/
    yylval = lval;
%}

"/*" { BEGIN(COMMENT); }
<COMMENT>"*/" { BEGIN(INITIAL); }
<COMMENT>(.|\n) ;
<COMMENT><<EOF>> { /*err*/ }

[ \t]+ {
}

[\n]|[\r\n] {
    location->lines();
}

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval->build< std::string >(yytext);
    // a plain identifier
    // switch(driver.identifierType(yytext)) {
    // this could be something else too. Could be an existing variable, for example .TDO: handle that
    // }
    return token::NEW_IDENTIFIER;
}

[a-zA-Z_][a-zA-Z0-9_]*\. {
    BEGIN(NESTED_IDENTIFIER);

    ident_lookup_stack = std::stack<const rosewood::DeclarationContext*>{};
    auto decl = driver.getDeclaration(std::string_view(yytext, yyleng - 1));
    if (!decl || !decl->asDeclContext()) {
        std::cerr << fmt::format("Error: identifier '{}' is not a top level declaration. Going to crash.. \n", std::string_view(yytext, yyleng - 1));
    }
    ident_lookup_stack.push(decl->asDeclContext());
}

<NESTED_IDENTIFIER>[a-zA-Z_][a-zA-Z0-9_]* {
    auto parentDecl = ident_lookup_stack.top();
    auto decl = parentDecl->getDeclaration(std::string_view(yytext, yyleng));
    if (!decl) {
        std::cerr << fmt::format("Error: identifier '{}' is not a declaration. Going to crash.. \n", std::string_view(yytext, yyleng - 1));
    }

    BEGIN(INITIAL);
    if (decl->asTypeDeclaration()) {
        yylval->build<moose::ast::TypeRef>(moose::ast::TypeRef{decl->asTypeDeclaration()});
        return token::TYPENAME;
    }

    yylval->build< std::string >(yytext);
    return token::UNDEFINED_QUALNAME;
}

<NESTED_IDENTIFIER>[a-zA-Z_][a-zA-Z0-9_]*\. {
    auto parentDecl = ident_lookup_stack.top();
    auto decl = parentDecl->getDeclaration(std::string_view(yytext, yyleng - 1));
    if (!decl || !decl->asDeclContext()) {
        std::cerr << fmt::format("Error: identifier '{}' is not a declaration context. Going to crash.. \n", std::string_view(yytext, yyleng - 1));
    }
    ident_lookup_stack.push(decl->asDeclContext());
}

[1-9][0-9]* {
    std::istringstream ss(yytext);
    std::int64_t v;
    ss >> v;
    yylval->build<std::int64_t>(v);
    return token::INTEGER_NUM;
}

((0x)|(0X))[1-9a-fA-F]+ {
    std::istringstream ss(yytext);
    std::int64_t v;
    ss >> std::hex>> v;
    yylval->build<std::int64_t>(v);
    return token::INTEGER_NUM;
}


([1-9][0-9]*)*\.[0-9]* {
    std::istringstream ss(yytext);
    double v;
    ss >> v;
    yylval->build<double>(v);
    return token::FLOATIN_NUM;
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
