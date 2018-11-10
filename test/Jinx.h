#pragma once

#include <cstdint>
#include <string>

namespace jinx {

    enum JinxTypes : int8_t {
        underJinx = -32,
        overUnderJinx,

        zeroJinx = 0,
        moreJinx,
        superJinx = 100
    };

    class Jinx {
    public:
        Jinx(const std::string& hisName)
            :name(hisName) {}

        int aMethod();

    private:
        std::string name;
    };

}
