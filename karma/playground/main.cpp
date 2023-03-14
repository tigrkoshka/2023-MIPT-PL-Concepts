#include <iostream>
#include <karma>

int main() {
    karma::compiler::MustCompile("../programs/04_factorial_loop.krm");

    karma::Executor executor;
    uint32_t ret_code = executor.MustExecute("../programs/04_factorial_loop.a");

    std::cout << "executable returned code " << ret_code;

    karma::disassembler::MustDisassemble("../programs/04_factorial_loop.a");

    return 0;
}
