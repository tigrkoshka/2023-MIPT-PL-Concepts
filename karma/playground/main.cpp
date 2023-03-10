#include <karma>

int main() {
    karma::compiler::MustCompile("../programs/04_factorial_loop.krm");

    karma::Executor executor;
    executor.MustExecute("../programs/04_factorial_loop.a");

    karma::disassembler::MustDisassemble("../programs/04_factorial_loop.a");

    return 0;
}
