#include <iostream>
#include <karma>

int main() {
    karma::Compiler::MustCompile("../programs/print/playground_uint32.krm");

    karma::Executor executor;
    uint32_t ret_code =
        executor.MustExecute("../programs/print/playground_uint32.a");

    std::cout << "executable returned code " << ret_code << std::endl;

    karma::Disassembler::MustDisassemble(
        "../programs/print/playground_uint32.a");

    return 0;
}
