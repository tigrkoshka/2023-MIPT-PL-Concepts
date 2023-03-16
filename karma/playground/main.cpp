#include <iostream>
#include <karma>

int main() {
    karma::Compiler::MustCompile("../programs/print/playground.krm");

    karma::Executor executor;
    uint32_t ret_code = executor.MustExecute("../programs/print/playground.a");

    std::cout << "executable returned code " << ret_code << std::endl;

    karma::Disassembler::MustDisassemble("../programs/print/playground.a");

    return 0;
}
