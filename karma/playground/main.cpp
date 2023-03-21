#include <iostream>
#include <karma>
#include <string>

class NoOpStreamBuff : public std::streambuf {
   public:
    int overflow(int num) override {
        return num;
    }
};

std::string Assembler(const std::string& filepath) {
    return filepath + ".krm";
}

std::string Exec(const std::string& filepath) {
    return filepath + ".a";
}

int main() {
    std::string filepath = "../programs/print/playgrounds/printf";

    NoOpStreamBuff no_op_stream_buff;
    std::ostream no_op_stream(&no_op_stream_buff);

    karma::Compiler::Compile(Assembler(filepath), "", no_op_stream);

    karma::Executor executor;
    uint32_t ret_code = executor.Execute(Exec(filepath), no_op_stream);

    std::cout << "executable returned code " << ret_code << std::endl;

    karma::Disassembler::Disassemble(Exec(filepath), "", no_op_stream);

    return 0;
}
