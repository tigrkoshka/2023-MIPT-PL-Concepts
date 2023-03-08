#include "compiler.hpp"
#include "disassembler.hpp"
#include "executor.hpp"

int main() {
    karma::MustCompile("../docs/samples/04_factorial_recursion.krm");

    karma::Executor executor;
    executor.MustExecute("../docs/samples/04_factorial_recursion.a");

    karma::MustDisassemble("../docs/samples/04_factorial_recursion.a");

    return 0;
}
