#include "BasicDefinitions.h"


void basic::PlainClass::noArgsNoReturnMethod() {

}

int basic::PlainClass::doubleInteger(int param) const {
    return param * 2;
}

void basic::PlainClass::overloadedMethod(int) {

}

void basic::PlainClass::overloadedMethod(void) {

}

void basic::PlainClass::constNoExceptFunction() const noexcept {
}

int basic::PlainClass::fct() noexcept {
	return 12;
}
