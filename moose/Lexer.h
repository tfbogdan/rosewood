#pragma once

#include <istream>
#include <stack>

#include <parser.hpp>
#include <rosewood/runtime.hpp>

#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "driver.h"

namespace moose {


    class Lexer : public yyFlexLexer {
    public:
        Lexer(std::istream *in, Driver &drv)
            : yyFlexLexer(in),
            driver(drv) {}

        using FlexLexer::yylex;
        virtual int yylex(  moose::Parser::semantic_type * const lval,
                            moose::Parser::location_type *location );

     private:
        moose::Parser::semantic_type *yylval = nullptr;
        Driver &driver;

        std::stack<const rosewood::DeclarationContext*> ident_lookup_stack;
    };

}
