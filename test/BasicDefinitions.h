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
    PlainClass() = default;
    PlainClass(const PlainClass&) = default;
    PlainClass(PlainClass&&) noexcept = default;

    void noArgsNoReturnMethod();

    int doubleInteger(int namedParam) const;

    void overloadedMethod();
    void constNoExceptFunction() const noexcept;
    void overloadedMethod(int i);

    static int fct() noexcept;

    float floatField = .2f;
    int intField = 23;

    enum innerEnum{
        firstEnum, secondOne, third = -3
    };

private:
    float privateFloatMember = 3;
};

}
