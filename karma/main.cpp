#include "compiler.hpp"
#include "disassembler.hpp"
#include "executor.hpp"

int main() {
    karma::MustCompile("../programs/04_factorial_loop.krm");

    karma::Executor executor;
    executor.MustExecute("../programs/04_factorial_loop.a");

    karma::MustDisassemble("../programs/04_factorial_loop.a");

    return 0;
}
