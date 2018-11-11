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

    typedef enum {
        firstEnum, secondOne, third = -3
    } UntypedEnum;

    class Jinx {
    public:
        Jinx(const std::string& hisName)
            :name(hisName) {}


        int aMethod();

        void overloadedMethod();
        void overloadedMethod(int i);

        float aField;

        enum innerEnum{
            firstEnum, secondOne, third = -3
        };


    private:
        std::string name;
    };

}
