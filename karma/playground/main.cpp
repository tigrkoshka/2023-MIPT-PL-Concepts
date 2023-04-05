#include <iostream>
#include <karma>
#include <string>

std::string Assembler(const std::string& filepath) {
    return filepath + ".krm";
}

std::string Exec(const std::string& filepath) {
    return filepath + ".a";
}

std::string Disassembled(const std::string& filepath) {
    return filepath + "_disassembled.krm";
}

std::string DisassembledExec(const std::string& filepath) {
    return filepath + "_disassembled.a";
}

int main() {
    const std::string filepath = "../programs/print/playgrounds/printf";

    karma::Compiler::Compile(Assembler(filepath), std::cerr);

    karma::Executor executor(karma::Executor::Config::Strict());
    const uint32_t ret_code = executor.Execute(Exec(filepath), std::cerr);

    std::cout << "executable returned code " << ret_code << std::endl;

    karma::Disassembler::Disassemble(Exec(filepath), karma::Logger(std::cerr));

    // disassembly testing

    /*
    karma::Compiler::Compile(Disassembled(filepath));

    ret_code = executor.Execute(DisassembledExec(filepath));

    std::cout << "executable returned code " << ret_code << std::endl;
    */

    return 0;
}
