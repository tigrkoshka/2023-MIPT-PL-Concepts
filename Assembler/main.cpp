#include <iostream>

#include "karma/compiler.hpp"
#include "karma/disassembler.hpp"
#include "karma/executor.hpp"

int main() {
    karma::Compiler::MustCompile("../docs/samples/square_no_function.krm");

    karma::Executor executor;
    executor.MustExecute("../docs/samples/square_no_function");

    karma::Disassembler::MustDisassemble("../docs/samples/square_no_function");

    return 0;
}
