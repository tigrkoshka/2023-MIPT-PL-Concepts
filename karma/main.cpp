#include "src/compiler.hpp"
#include "src/disassembler.hpp"
#include "src/executor.hpp"

int main() {
    karma::Compiler::MustCompile("../docs/samples/01_square_no_function.krm");

    karma::Executor executor;
    executor.MustExecute("../docs/samples/01_square_no_function.a");

    karma::Disassembler::MustDisassemble(
        "../docs/samples/01_square_no_function.a");

    return 0;
}
