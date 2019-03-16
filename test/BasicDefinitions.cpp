#include "BasicDefinitions.h"


#include <iostream>


void basic::PlainClass::noArgsNoReturnMethod() {

}

int basic::PlainClass::doubleInteger(int param) {
    return param * 2;
}

void basic::PlainClass::overloadedMethod(int) {

}

void basic::PlainClass::overloadedMethod(void) {

}

void basic::PlainClass::constNoExceptFunction() const noexcept {
    std::cerr << "basic::PlainClass::constNoExceptFunction()\n";
}
