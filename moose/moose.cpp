#include <moose/moose.hpp>
#include <parser.hpp>
#include <Lexer.h>
#include <iostream>
#include <driver.h>
namespace moose {

    Engine::Engine(rosewood::Index &idx)
        :index(idx) {

    }

    int moose::Engine::start_stdin() {
        moose::Driver driver(index);

        moose::Lexer lexer(&std::cin, driver);
        moose::Parser parser(lexer, driver);
        return parser.parse();
    }

}
