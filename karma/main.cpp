#include "compiler.hpp"
#include "disassembler.hpp"
#include "executor.hpp"

int main() {
    karma::MustCompile("../docs/samples/01_square_no_function.krm");

    karma::Executor executor;
    executor.MustExecute("../docs/samples/01_square_no_function.a");

    karma::MustDisassemble(
        "../docs/samples/01_square_no_function.a");

    return 0;
}
