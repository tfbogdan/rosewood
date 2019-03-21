#pragma once

#include <rosewood/index.hpp>

namespace moose {

    class Engine {
    public:
        Engine(rosewood::Index& idx);
        int start_stdin();

    private:
        rosewood::Index& index;
    };
}
