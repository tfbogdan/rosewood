#include <moose/moose.hpp>
#include <parser.hpp>
#include <Lexer.h>
#include <iostream>

namespace moose {

    Engine::Engine(rosewood::Index &idx)
        :index(idx) {

    }

    int moose::Engine::start_stdin() {
        moose::Lexer lexer(&std::cin);
        moose::Parser parser(lexer);

        return parser.parse();
    }

}
