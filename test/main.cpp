#include "Jinx.h"
#include "Jinx.metadata.h"

#include <mc/MetaType.h>
#include <iostream>
#include <functional>

template<typename MetaType>
void printEnumerators([[maybe_unused]] MetaType metaType) {
    using enumeratorsType = typename MetaType::enumerators;
    std::cout << "enum " << MetaType::name << ":\n";
    std::apply([](auto &&...vals){
        ((std::cout << "  " << vals.name << ": " << vals.value << "\n"), ...);
    }, enumeratorsType());
}

int main() {
    printEnumerators(mc::Jinx::jinx::JinxTypes());
    static_assert (mc::has_method(mc::Jinx::jinx::Jinx(), "aMethod"), "class jinx::Jinx should have a method called aMethod.");
    static_assert (!mc::has_method(mc::Jinx::jinx::Jinx(), "fictionalMethod"), "class jinx::Jinx isn't support to have a method named fictionalMethod.");

    return 0;
}
