#pragma once


namespace basic {

enum Enum {
    negativeEnumerator = -32,
    nextNegativeEnumerator,

    zeroEnumerator = 0,
    oneEnumerator,
    hundredEnumerator = 100
};

typedef enum {
    firstEnum, secondOne, third = -3
} UnnamedEnum;

struct podStruct {
    int intFiled;
    long longField;
    char charField;
};

class PlainClass {
public:
    void noArgsNoReturnMethod();

    int doubleInteger(int namedParam);

    void overloadedMethod();
    void overloadedMethod(int i);

    float floatField;

    enum innerEnum{
        firstEnum, secondOne, third = -3
    };

private:
    float privateFloatMember;
};

}
