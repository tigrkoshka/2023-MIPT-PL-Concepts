#include "compiler.hpp"
#include "disassembler.hpp"
#include "executor.hpp"

int main() {
    karma::MustCompile("../docs/samples/01_hello_world.krm");

    karma::Executor executor;
    executor.MustExecute("../docs/samples/01_hello_world.a");

    karma::MustDisassemble("../docs/samples/01_hello_world.a");

    return 0;
}
