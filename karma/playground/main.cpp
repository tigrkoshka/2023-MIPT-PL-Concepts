#include <iostream>
#include <karma>
#include <string>

std::string Assembler(const std::string& filepath) {
    return filepath + ".krm";
}

std::string Exec(const std::string& filepath) {
    return filepath + ".a";
}

int main() {
    std::string filepath = "../programs/print/playgrounds/printf";

    karma::Compiler::Compile(Assembler(filepath), std::clog);

    karma::Executor executor(karma::Executor::Config::Strict());
    uint32_t ret_code = executor.Execute(Exec(filepath), std::clog);

    std::cout << "executable returned code " << ret_code << std::endl;

    karma::Disassembler::Disassemble(Exec(filepath), karma::Logger(std::clog));

    return 0;
}
