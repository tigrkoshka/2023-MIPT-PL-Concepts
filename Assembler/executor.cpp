#include "executor.hpp"

#include <cmath>        // for floor
#include <exception>    // for exception
#include <fstream>      // for ifstream
#include <iomanip>      // for quoted
#include <iostream>     // for cout
#include <sstream>      // for ostringstream
#include <stdexcept>    // for runtime_error
#include <string>       // for string
#include <type_traits>  // for make_signed_t

#include "specs/architecture.hpp"
#include "specs/exec.hpp"
#include "specs/flags.hpp"

namespace karma {

namespace flags = detail::specs::flags;
namespace arch  = detail::specs::arch;
namespace types = arch::types;

namespace cmd     = detail::specs::cmd;
namespace args    = cmd::args;
namespace syscall = cmd::syscall;

namespace exec = detail::specs::exec;

////////////////////////////////////////////////////////////////////////////////
///                                  Errors                                  ///
////////////////////////////////////////////////////////////////////////////////

struct Executor::Error : std::runtime_error {
   protected:
    explicit Error(const std::string& message)
        : std::runtime_error(message) {}

   public:
    Error(const Error&)            = default;
    Error& operator=(const Error&) = default;
    Error(Error&&)                 = default;
    Error& operator=(Error&&)      = default;
    ~Error() override              = default;
};

struct Executor::InternalError : Error {
   private:
    friend void Executor::Execute(const std::string& exec_path);

   private:
    explicit InternalError(const std::string& message)
        : Error("internal executor error: " + message) {}

   public:
    InternalError(const InternalError&)            = default;
    InternalError& operator=(const InternalError&) = default;
    InternalError(InternalError&&)                 = default;
    InternalError& operator=(InternalError&&)      = default;
    ~InternalError() override                      = default;

   public:
    static InternalError UnknownCommandFormat(cmd::Format format) {
        std::ostringstream ss;
        ss << "unknown command format " << format;
        return InternalError{ss.str()};
    }

    static InternalError UnknownCommandForFormat(cmd::Format format,
                                                 cmd::Code code) {
        std::ostringstream ss;
        ss << "unknown command code " << code << " for command format "
           << std::quoted(cmd::kFormatToString.at(format));
        return InternalError{ss.str()};
    }
};

struct Executor::ExecutionError : Error {
   private:
    explicit ExecutionError(const std::string& message)
        : Error("execution error" + message) {}

   public:
    ExecutionError(const ExecutionError&)            = default;
    ExecutionError& operator=(const ExecutionError&) = default;
    ExecutionError(ExecutionError&&)                 = default;
    ExecutionError& operator=(ExecutionError&&)      = default;
    ~ExecutionError() override                       = default;

   public:
    static ExecutionError UnknownCommand(cmd::Code code) {
        std::ostringstream ss;
        ss << "unknown command code " << code;
        return ExecutionError{ss.str()};
    }

    static ExecutionError UnknownSyscallCode(syscall::Code code) {
        std::ostringstream ss;
        ss << "unknown syscall code" << code;
        return ExecutionError{ss.str()};
    }

    template <typename T>
        requires std::is_same_v<T, types::TwoWords> ||
                 std::is_same_v<T, types::Double>
    static ExecutionError DivisionByZero(T dividend, T divisor) {
        std::ostringstream ss;
        ss << "a division by zero occurred when dividing " << dividend << " by "
           << divisor;
        return ExecutionError{ss.str()};
    }

    static ExecutionError QuotientOverflow(types::TwoWords dividend,
                                           types::TwoWords divisor) {
        std::ostringstream ss;
        ss << "a quotient overflow occurred when dividing " << dividend
           << " by " << divisor;
        return ExecutionError{ss.str()};
    }

    static ExecutionError DtoiOverflow(types::Double value) {
        std::ostringstream ss;
        ss << "a word overflow occurred when casting " << value
           << " to integer";
        return ExecutionError{ss.str()};
    }

    static ExecutionError InvalidPutCharValue(args::Immediate value) {
        std::ostringstream ss;
        ss << "the value in the register for the PUTCHAR syscall is " << value
           << ", which is an invalid char, because it is greater than 255";
        return ExecutionError{ss.str()};
    }

    // TODO: use this
    static ExecutionError AddressOutsideOfMemory(args::Address address) {
        std::ostringstream ss;
        ss << "address " << std::hex << address
           << " is outside of memory (size " << arch::kMemorySize << ")";
        return ExecutionError{ss.str()};
    }
};

struct Executor::ExecFileError : Error {
   private:
    explicit ExecFileError(const std::string& message)
        : Error("exec file error: " + message) {}

   public:
    ExecFileError(const ExecFileError&)            = default;
    ExecFileError& operator=(const ExecFileError&) = default;
    ExecFileError(ExecFileError&&)                 = default;
    ExecFileError& operator=(ExecFileError&&)      = default;
    ~ExecFileError() override                      = default;

   public:
    static ExecFileError FailedToOpen() {
        std::ostringstream ss;
        ss << "failed to open exec file, please check that the path is correct";
        return ExecFileError{ss.str()};
    }

    static ExecFileError TooSmallForHeader(size_t size) {
        std::ostringstream ss;
        ss << "exec size is " << size << ", which is less than "
           << exec::kHeaderSize << " bytes required for the header";
        return ExecFileError{ss.str()};
    }

    static ExecFileError InvalidExecSize(size_t exec_size,
                                         size_t code_size,
                                         size_t consts_size,
                                         size_t data_size) {
        std::ostringstream ss;
        ss << "the exec file size (" << exec_size << ") does not equal "
           << exec::kHeaderSize + code_size + consts_size + data_size
           << ", which is the sum of the header size (" << exec::kHeaderSize
           << ") and the total of the sizes of the code segment (" << code_size
           << "), the constants segment (" << consts_size
           << ") and the data segment (" << data_size
           << ") specified in the header";
        return ExecFileError{ss.str()};
    }

    static ExecFileError InvalidIntroString(const std::string& intro) {
        std::ostringstream ss;
        ss << "expected the welcoming " << std::quoted(exec::kIntroString)
           << " string (and a trailing \\0) as the first " << exec::kIntroSize
           << " bytes, instead got: " << std::quoted(intro);
        return ExecFileError{ss.str()};
    }

    static ExecFileError InvalidProcessorID(types::Word processor_id) {
        std::ostringstream ss;
        ss << "exec file is built for processor with ID " << processor_id
           << ", current processor ID: " << exec::kProcessorID;
        return ExecFileError{ss.str()};
    }
};

////////////////////////////////////////////////////////////////////////////////
///                                   Utils                                  ///
////////////////////////////////////////////////////////////////////////////////

types::Double Executor::ToDbl(types::TwoWords ull) {
    return *reinterpret_cast<types::Double*>(&ull);
}

types::TwoWords Executor::ToUll(types::Double dbl) {
    return *reinterpret_cast<types::TwoWords*>(&dbl);
}

types::TwoWords Executor::GetTwoRegisters(args::Source lower) const {
    return (static_cast<types::TwoWords>(registers_[lower + 1])
            << types::kWordSize) +
           static_cast<types::TwoWords>(registers_[lower]);
}

void Executor::PutTwoRegisters(types::TwoWords value, args::Receiver lower) {
    registers_[lower]     = static_cast<types::Word>(value);
    registers_[lower + 1] = static_cast<types::Word>(value >> types::kWordSize);
}

template <std::totally_ordered T>
void Executor::WriteComparisonToFlags(T lhs, T rhs) {
    auto cmp_res = lhs <=> rhs;

    if (cmp_res == 0) {
        flags_ = flags::kEqual;
    } else if (cmp_res > 0) {
        flags_ = flags::kGreater;
    } else {
        // condition "cmp_res < 0" is true
        // because lhs and rhs are comparable
        // because of the template type constraint
        flags_ = flags::kLess;
    }
}

void Executor::Jump(flags::Flag flag, args::Address dst) {
    if ((flags_ & flag) != 0) {
        registers_[arch::kInstructionRegister] = dst;
    }
}

////////////////////////////////////////////////////////////////////////////////
///                             Separate commands                            ///
////////////////////////////////////////////////////////////////////////////////

void Executor::Divide(types::TwoWords lhs,
                      types::TwoWords rhs,
                      args::Receiver recv) {
    if (rhs == 0) {
        throw ExecutionError::DivisionByZero(lhs, rhs);
    }

    types::TwoWords quotient = lhs / rhs;

    if (quotient > static_cast<types::TwoWords>(types::kMaxWord)) {
        throw ExecutionError::QuotientOverflow(lhs, rhs);
    }

    types::TwoWords remainder = lhs % rhs;

    registers_[recv]     = static_cast<types::Word>(quotient);
    registers_[recv + 1] = static_cast<types::Word>(remainder);
}

bool Executor::Syscall(args::Register reg, syscall::Code code) {
    switch (code) {
        case syscall::EXIT: {
            return false;
        }

        case syscall::SCANINT: {
            std::make_signed_t<types::Word> val{};
            std::cin >> val;

            registers_[reg] = static_cast<types::Word>(val);
            break;
        }

        case syscall::SCANDOUBLE: {
            types::Double val{};
            std::cin >> val;

            PutTwoRegisters(ToUll(val), reg);
            break;
        }

        case syscall::PRINTINT: {
            std::cout << static_cast<std::make_signed_t<types::Word>>(
                registers_[reg]);
            break;
        }

        case syscall::PRINTDOUBLE: {
            std::cout << ToDbl(GetTwoRegisters(reg));
            break;
        }

        case syscall::GETCHAR: {
            types::Char val{};
            std::cin >> val;

            registers_[reg] = static_cast<types::Word>(val);
            break;
        }

        case syscall::PUTCHAR: {
            if (registers_[reg] > types::kMaxChar) {
                throw ExecutionError::InvalidPutCharValue(registers_[reg]);
            }

            std::cout << static_cast<types::Char>(registers_[reg]);
            break;
        }

        default: {
            throw ExecutionError::UnknownSyscallCode(code);
        }
    }

    return true;
}

void Executor::Push(types::Word value) {
    memory_[registers_[arch::kStackRegister]--] = value;
}

void Executor::Pop(args::Receiver recv, args::Modifier mod) {
    registers_[recv] = memory_[++registers_[arch::kStackRegister]] + mod;
}

arch::Address Executor::Call(args::Address callee) {
    arch::Address ret = registers_[arch::kInstructionRegister];

    Push(registers_[arch::kInstructionRegister]);
    registers_[arch::kInstructionRegister] = callee;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
///                              Execute command                             ///
////////////////////////////////////////////////////////////////////////////////

bool Executor::ExecuteRMCommand(cmd::Code code,
                                args::Register reg,
                                args::Address addr) {
    switch (code) {
        case cmd::LOAD: {
            registers_[reg] = memory_[addr];
            break;
        }

        case cmd::STORE: {
            memory_[addr] = registers_[reg];
            break;
        }

        case cmd::LOAD2: {
            registers_[reg]     = memory_[addr];
            registers_[reg + 1] = memory_[addr + 1];
            break;
        }

        case cmd::STORE2: {
            memory_[addr]     = registers_[reg];
            memory_[addr + 1] = registers_[reg + 1];
            break;
        }

        default: {
            throw InternalError::UnknownCommandForFormat(cmd::RM, code);
        }
    }

    return true;
}

bool Executor::ExecuteRRCommand(cmd::Code code,
                                args::Receiver recv,
                                args::Source src,
                                args::Modifier mod) {
    auto lhs_word = [this, recv]() -> types::Word {
        return registers_[recv];
    };

    auto lhs_two_words = [this, recv]() -> types::TwoWords {
        return GetTwoRegisters(recv);
    };

    auto lhs_dbl = [&lhs_two_words]() -> types::Double {
        return ToDbl(lhs_two_words());
    };

    auto rhs_word = [this, src, mod]() -> types::Word {
        return registers_[src] + mod;
    };

    auto rhs_two_words = [this, src, mod]() -> types::TwoWords {
        return GetTwoRegisters(src) + static_cast<types::TwoWords>(mod);
    };

    auto rhs_dbl = [&rhs_two_words]() -> types::Double {
        return ToDbl(rhs_two_words());
    };

    switch (code) {
        case cmd::ADD: {
            registers_[recv] += rhs_word();
            break;
        }

        case cmd::SUB: {
            registers_[recv] -= rhs_word();
            break;
        }

        case cmd::MUL: {
            types::TwoWords res = static_cast<types::TwoWords>(lhs_word()) *
                                  static_cast<types::TwoWords>(rhs_word());
            PutTwoRegisters(res, recv);
            break;
        }

        case cmd::DIV: {
            Divide(lhs_two_words(),
                   static_cast<types::TwoWords>(rhs_word()),
                   recv);
            break;
        }

        case cmd::SHL: {
            registers_[recv] <<= rhs_word();
            break;
        }

        case cmd::SHR: {
            registers_[recv] >>= rhs_word();
            break;
        }

        case cmd::AND: {
            registers_[recv] &= rhs_word();
            break;
        }

        case cmd::OR: {
            registers_[recv] |= rhs_word();
            break;
        }

        case cmd::XOR: {
            registers_[recv] ^= rhs_word();
            break;
        }

        case cmd::MOV: {
            registers_[recv] = rhs_word();
            break;
        }

        case cmd::ADDD: {
            PutTwoRegisters(ToUll(lhs_dbl() + rhs_dbl()), recv);
            break;
        }

        case cmd::SUBD: {
            PutTwoRegisters(ToUll(lhs_dbl() - rhs_dbl()), recv);
            break;
        }

        case cmd::MULD: {
            PutTwoRegisters(ToUll(lhs_dbl() * rhs_dbl()), recv);
            break;
        }

        case cmd::DIVD: {
            types::Double lhs = lhs_dbl();
            types::Double rhs = rhs_dbl();

            if (rhs == 0) {
                throw ExecutionError::DivisionByZero(lhs, rhs);
            }

            PutTwoRegisters(ToUll(lhs / rhs), recv);
            break;
        }

        case cmd::ITOD: {
            PutTwoRegisters(ToUll(static_cast<types::Double>(rhs_word())),
                            recv);
            break;
        }

        case cmd::DTOI: {
            types::Double dbl = rhs_dbl();
            auto res          = static_cast<types::TwoWords>(floor(dbl));

            if (res >= static_cast<types::TwoWords>(types::kMaxWord)) {
                throw ExecutionError::DtoiOverflow(dbl);
            }

            registers_[recv] = static_cast<types::Word>(res);
            break;
        }

        case cmd::CALL: {
            registers_[recv] = Call(rhs_word());
            break;
        }

        case cmd::CMP: {
            WriteComparisonToFlags(lhs_word(), rhs_word());
            break;
        }

        case cmd::CMPD: {
            WriteComparisonToFlags(lhs_dbl(), rhs_dbl());
            break;
        }

        case cmd::LOADR: {
            registers_[recv] = memory_[rhs_word()];
            break;
        }

        case cmd::STORER: {
            memory_[rhs_word()] = registers_[recv];
            break;
        }

        case cmd::LOADR2: {
            args::Address address = rhs_word();

            registers_[recv]     = memory_[address];
            registers_[recv + 1] = memory_[address + 1];
            break;
        }

        case cmd::STORER2: {
            args::Address address = rhs_word();

            memory_[address]     = registers_[recv];
            memory_[address + 1] = registers_[recv + 1];
            break;
        }

        default: {
            throw InternalError::UnknownCommandForFormat(cmd::RR, code);
        }
    }

    return true;
}

bool Executor::ExecuteRICommand(cmd::Code code,
                                args::Register reg,
                                args::Immediate imm) {
    switch (code) {
        case cmd::HALT: {
            registers_[reg] = imm;
            return false;
        }

        case cmd::SYSCALL: {
            if (!Syscall(reg, static_cast<syscall::Code>(imm))) {
                return false;
            }
            break;
        }

        case cmd::ADDI: {
            registers_[reg] += imm;
            break;
        }

        case cmd::SUBI: {
            registers_[reg] -= imm;
            break;
        }

        case cmd::MULI: {
            auto res = static_cast<types::TwoWords>(registers_[reg]) * imm;

            PutTwoRegisters(res, reg);
            break;
        }

        case cmd::DIVI: {
            Divide(GetTwoRegisters(reg),
                   static_cast<types::TwoWords>(imm),
                   reg);
            break;
        }

        case cmd::LC: {
            registers_[reg] = imm;
            break;
        }

        case cmd::SHLI: {
            registers_[reg] <<= imm;
            break;
        }

        case cmd::SHRI: {
            registers_[reg] >>= imm;
            break;
        }

        case cmd::ANDI: {
            registers_[reg] &= imm;
            break;
        }

        case cmd::ORI: {
            registers_[reg] |= imm;
            break;
        }

        case cmd::XORI: {
            registers_[reg] ^= imm;
            break;
        }

        case cmd::NOT: {
            registers_[reg] = ~registers_[reg];
            break;
        }

        case cmd::PUSH: {
            Push(registers_[reg] + imm);
            break;
        }

        case cmd::POP: {
            Pop(reg, imm);
            break;
        }

        case cmd::CMPI: {
            WriteComparisonToFlags(registers_[reg], imm);
            break;
        }

        default: {
            throw InternalError::UnknownCommandForFormat(cmd::RI, code);
        }
    }

    return true;
}

bool Executor::ExecuteJCommand(cmd::Code code, args::Address addr) {
    switch (code) {
        case cmd::CALLI: {
            Call(addr);
            break;
        }

        case cmd::RET: {
            registers_[arch::kStackRegister] += addr;
            Pop(arch::kInstructionRegister, 0);
            break;
        }

        case cmd::JMP: {
            registers_[arch::kInstructionRegister] = addr;
            break;
        }

        case cmd::JEQ: {
            Jump(flags::EQUAL, addr);
            break;
        }

        case cmd::JNE: {
            Jump(flags::NOT_EQUAL, addr);
            break;
        }

        case cmd::JG: {
            Jump(flags::GREATER, addr);
            break;
        }

        case cmd::JL: {
            Jump(flags::LESS, addr);
            break;
        }

        case cmd::JGE: {
            Jump(flags::GREATER_OR_EQUAL, addr);
            break;
        }

        case cmd::JLE: {
            Jump(flags::LESS_OR_EQUAL, addr);
            break;
        }

        default: {
            throw InternalError::UnknownCommandForFormat(cmd::J, code);
        }
    }

    return true;
}

bool Executor::ExecuteCommand(cmd::Bin command) {
    registers_[arch::kInstructionRegister]++;

    auto code = cmd::GetCode(command);
    if (!cmd::kCodeToFormat.contains(code)) {
        throw ExecutionError::UnknownCommand(code);
    }

    switch (cmd::Format format = cmd::kCodeToFormat.at(code)) {
        case cmd::RM: {
            auto [reg, addr] = cmd::parse::RM(command);
            return ExecuteRMCommand(code, reg, addr);
        }

        case cmd::RR: {
            auto [recv, src, mod] = cmd::parse::RR(command);
            return ExecuteRRCommand(code, recv, src, mod);
        }

        case cmd::RI: {
            auto [reg, imm] = cmd::parse::RI(command);
            return ExecuteRICommand(code, reg, imm);
        }

        case cmd::J: {
            auto [addr] = cmd::parse::J(command);
            return ExecuteJCommand(code, addr);
        }

        default: {
            throw InternalError::UnknownCommandFormat(format);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
///                              Execute binary                              ///
////////////////////////////////////////////////////////////////////////////////

void Executor::ExecuteImpl(const std::string& exec_path) {
    // set the input position indicator to the end (using std::ios::ate)
    // after opening the file to get the size of the exec file,
    // which is used later to verify that the exec is not malformed
    std::ifstream binary(exec_path, std::ios::binary | std::ios::ate);

    // check that the file was found
    if (binary.fail()) {
        throw ExecFileError::FailedToOpen();
    }

    // check that the exec size is not too small to contain a valid header
    auto exec_size = static_cast<size_t>(binary.tellg());
    if (exec_size < exec::kHeaderSize) {
        throw ExecFileError::TooSmallForHeader(exec_size);
    }

    // reset input position indicator
    binary.seekg(0);

    auto read_word = [&binary]() -> types::Word {
        types::Word word{};
        binary.read(reinterpret_cast<char*>(&word), 4);
        return word;
    };

    // read the first 16 bytes, which should represent
    // the introductory string (including the final '\0')
    std::string intro(exec::kIntroSize, '\0');
    binary.read(intro.data(), static_cast<std::streamsize>(exec::kIntroSize));

    // check that the introductory string is valid
    if (intro != exec::kIntroString) {
        throw ExecFileError::InvalidIntroString(intro);
    }

    // read the code, constants and data segments sizes (in bytes)
    size_t code_size   = read_word();
    size_t consts_size = read_word();
    size_t data_size   = read_word();

    // check that the exec file size equals the size specified by the header
    if (exec_size != exec::kHeaderSize + code_size + consts_size + data_size) {
        throw ExecFileError::InvalidExecSize(exec_size,
                                             code_size,
                                             consts_size,
                                             data_size);
    }

    // read the address of the first instruction
    registers_[arch::kInstructionRegister] = read_word();

    // read the initial stack pointer value
    registers_[arch::kStackRegister] = read_word();

    // read the target processor ID
    types::Word processor_id = read_word();
    if (processor_id != exec::kProcessorID) {
        throw ExecFileError::InvalidProcessorID(processor_id);
    }

    // jump to the code segment
    binary.seekg(exec::kCodeSegmentPos);

    // the code_size is denoted in bytes, so we need to divide
    // it by 4 to get the number of machine words
    size_t n_commands = code_size / types::kWordSize;

    // copy the code to virtual memory
    for (size_t i = 0; i < n_commands; ++i) {
        memory_[i] = read_word();
    }

    // execute while we are inside the code segment
    // and no command has returned an exit signal
    while (registers_[arch::kInstructionRegister] < n_commands &&
           ExecuteCommand(memory_[registers_[arch::kInstructionRegister]])) {
    }
}

void Executor::Execute(const std::string& exec_path) {
    try {
        ExecuteImpl(exec_path);
    } catch (const Error& e) {
        std::cout << exec_path << ": " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cout << exec_path << ": " << InternalError(e.what()).what()
                  << std::endl;
    } catch (...) {
        std::cout << exec_path << ": "
                  << InternalError("unexpected exception").what() << std::endl;
    }
}

}  // namespace karma
