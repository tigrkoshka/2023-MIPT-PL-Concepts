#include <iostream>

#include "compiler.hpp"
#include "disassembler.hpp"
#include "executor.hpp"

int main() {
    karma::Compiler::Compile("../samples/factorial_no_recursion.krm");

    karma::Executor executor;
    executor.Execute("../samples/factorial_no_recursion");

    karma::Disassembler::Disassemble("../samples/factorial_no_recursion");

    return 0;
}
