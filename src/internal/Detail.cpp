#include <iostream>
#include "internal/Detail.h"

void Detail::f() const {
    std::cout << "private function f: " << x << "\n";
}

// cppcheck-suppress unusedFunction
void Detail::g() {
    ++y;
    f();
    std::cout << "public function g: " << y << "\n";
}
