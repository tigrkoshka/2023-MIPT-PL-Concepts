#include <iostream>

#include "executor.hpp"

int main() {
    uint32_t a = 5;
    uint64_t b = 6;

    std::cout << a + b;

    karma::Executor executor;
    executor.Execute("x.txt");

    return 0;
}
