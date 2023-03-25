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
    std::string filepath = "../programs/print/playgrounds/printf";

    karma::Compiler::Compile(Assembler(filepath));

    karma::Executor executor(karma::Executor::Config::Strict());
    uint32_t ret_code = executor.Execute(Exec(filepath));

    std::cout << "executable returned code " << ret_code << std::endl;

    karma::Disassembler::Disassemble(Exec(filepath));

    // disassembly testing

    /*
    karma::Compiler::Compile(Disassembled(filepath));

    ret_code = executor.Execute(DisassembledExec(filepath));

    std::cout << "executable returned code " << ret_code << std::endl;
    */

    return 0;
}
