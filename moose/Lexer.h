#pragma once

#include <istream>
#include <parser.hpp>

#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

namespace moose {


    class Lexer : public yyFlexLexer {
    public:
        Lexer(std::istream *in)
            : yyFlexLexer(in),
            loc(new moose::Parser::location_type()) {}

        //get rid of override virtual function warning
        using FlexLexer::yylex;

        virtual int yylex(  moose::Parser::semantic_type * const lval,
                            moose::Parser::location_type *location );

     private:
        moose::Parser::semantic_type *yylval = nullptr;
        moose::Parser::location_type *loc    = nullptr;

    };

}
