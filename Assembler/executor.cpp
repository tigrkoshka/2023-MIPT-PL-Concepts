#include "executor.hpp"

#include <cmath>
#include <concepts>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace karma {

using namespace detail::specs;  // NOLINT(google-build-using-namespace)

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
    static InternalError UnknownCommandFormat(CommandFormat format) {
        std::ostringstream ss;
        ss << "unknown command format " << format;
        return InternalError{ss.str()};
    }

    static InternalError UnknownCommandForFormat(CommandFormat format,
                                                 CommandCode code) {
        std::ostringstream ss;
        ss << "unknown command code " << code << " for command format "
           << std::quoted(kFormatToString.at(format));
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
    static ExecutionError UnknownCommand(CommandCode code) {
        std::ostringstream ss;
        ss << "unknown command code " << code;
        return ExecutionError{ss.str()};
    }

    static ExecutionError UnknownSyscallCode(SyscallCode code) {
        std::ostringstream ss;
        ss << "unknown syscall code" << code;
        return ExecutionError{ss.str()};
    }

    template <typename T>
        requires std::is_same_v<T, TwoWords> || std::is_same_v<T, Double>
    static ExecutionError DivisionByZero(T dividend, T divisor) {
        std::ostringstream ss;
        ss << "a division by zero occurred when dividing " << dividend << " by "
           << divisor;
        return ExecutionError{ss.str()};
    }

    static ExecutionError QuotientOverflow(TwoWords dividend,
                                           TwoWords divisor) {
        std::ostringstream ss;
        ss << "a quotient overflow occurred when dividing " << dividend
           << " by " << divisor;
        return ExecutionError{ss.str()};
    }

    static ExecutionError DtoiOverflow(Double value) {
        std::ostringstream ss;
        ss << "a word overflow occurred when casting " << value
           << " to integer";
        return ExecutionError{ss.str()};
    }

    static ExecutionError InvalidPutCharValue(Immediate value) {
        std::ostringstream ss;
        ss << "the value in the register for the PUTCHAR syscall is " << value
           << ", which is an invalid char, because it is greater than 255";
        return ExecutionError{ss.str()};
    }

    static ExecutionError AddressOutsideOfMemory(Address address) {
        std::ostringstream ss;
        ss << "address " << std::hex << address
           << " is outside of memory (size " << kMemorySize << ")";
        return ExecutionError{ss.str()};
    }
};

struct Executor::ExecFileError : Error {
   private:
    explicit ExecFileError(const std::string& message)
        : Error("exec format error: " + message) {}

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

    static ExecFileError InvalidIntroString(const std::string& intro) {
        std::ostringstream ss;
        ss << "expected the welcoming " << std::quoted(exec::kIntroString)
           << " string (and a trailing \\0) as the first " << exec::kIntroSize
           << " bytes, instead got: " << std::quoted(intro);
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
};

////////////////////////////////////////////////////////////////////////////////
///                                   Utils                                  ///
////////////////////////////////////////////////////////////////////////////////

Double Executor::ToDbl(TwoWords ull) {
    return *reinterpret_cast<Double*>(&ull);
}

TwoWords Executor::ToUll(Double dbl) {
    return *reinterpret_cast<TwoWords*>(&dbl);
}

TwoWords Executor::GetTwoRegisters(Source lower) const {
    return (static_cast<TwoWords>(registers_[lower + 1]) << kWordSize) +
           static_cast<TwoWords>(registers_[lower]);
}

void Executor::PutTwoRegisters(TwoWords value, Receiver lower) {
    registers_[lower]     = static_cast<Word>(value);
    registers_[lower + 1] = static_cast<Word>(value >> kWordSize);
}

template <std::totally_ordered T>
void Executor::WriteComparisonToFlags(T lhs, T rhs) {
    auto cmp_res = lhs <=> rhs;

    if (cmp_res == 0) {
        flags_ = kEqual;
    } else if (cmp_res > 0) {
        flags_ = kGreater;
    } else {
        // condition "cmp_res < 0" is true
        // because lhs and rhs are comparable
        // because of the template type constraint
        flags_ = kLess;
    }
}

void Executor::Jump(Flag flag, Address dst) {
    if ((flags_ & flag) != 0) {
        registers_[kInstructionRegister] = dst;
    }
}

////////////////////////////////////////////////////////////////////////////////
///                             Separate commands                            ///
////////////////////////////////////////////////////////////////////////////////

void Executor::Divide(TwoWords lhs, TwoWords rhs, Receiver recv) {
    if (rhs == 0) {
        throw ExecutionError::DivisionByZero(lhs, rhs);
    }

    TwoWords quotient = lhs / rhs;

    if (quotient > static_cast<TwoWords>(kMaxWord)) {
        throw ExecutionError::QuotientOverflow(lhs, rhs);
    }

    TwoWords remainder = lhs % rhs;

    registers_[recv]     = static_cast<Word>(quotient);
    registers_[recv + 1] = static_cast<Word>(remainder);
}

bool Executor::Syscall(Register reg, SyscallCode code) {
    switch (code) {
        case EXIT: {
            return false;
        }

        case SCANINT: {
            std::make_signed_t<Word> val{};
            std::cin >> val;

            registers_[reg] = static_cast<Word>(val);
            break;
        }

        case SCANDOUBLE: {
            Double val{};
            std::cin >> val;

            PutTwoRegisters(ToUll(val), reg);
            break;
        }

        case PRINTINT: {
            std::cout << static_cast<std::make_signed_t<Word>>(registers_[reg]);
            break;
        }

        case PRINTDOUBLE: {
            std::cout << ToDbl(GetTwoRegisters(reg));
            break;
        }

        case GETCHAR: {
            Char val{};
            std::cin >> val;

            registers_[reg] = static_cast<Word>(val);
            break;
        }

        case PUTCHAR: {
            if (registers_[reg] > kMaxChar) {
                throw ExecutionError::InvalidPutCharValue(registers_[reg]);
            }

            std::cout << static_cast<Char>(registers_[reg]);
            break;
        }

        default: {
            throw ExecutionError::UnknownSyscallCode(code);
        }
    }

    return true;
}

void Executor::Push(Word value) {
    memory_[registers_[kStackRegister]--] = value;
}

void Executor::Pop(Receiver recv, Modifier mod) {
    registers_[recv] = memory_[++registers_[kStackRegister]] + mod;
}

Executor::Address Executor::Call(Address callee) {
    Address ret = registers_[kInstructionRegister];

    Push(registers_[kInstructionRegister]);
    registers_[kInstructionRegister] = callee;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
///                              Execute command                             ///
////////////////////////////////////////////////////////////////////////////////

bool Executor::ExecuteRMCommand(CommandCode code, Register reg, Address addr) {
    switch (code) {
        case LOAD: {
            registers_[reg] = memory_[addr];
            break;
        }

        case STORE: {
            memory_[addr] = registers_[reg];
            break;
        }

        case LOAD2: {
            registers_[reg]     = memory_[addr];
            registers_[reg + 1] = memory_[addr + 1];
            break;
        }

        case STORE2: {
            memory_[addr]     = registers_[reg];
            memory_[addr + 1] = registers_[reg + 1];
            break;
        }

        default: {
            throw InternalError::UnknownCommandForFormat(RM, code);
        }
    }

    return true;
}

bool Executor::ExecuteRRCommand(CommandCode code,
                                Receiver recv,
                                Source src,
                                Modifier mod) {
    auto lhs_word = [this, recv]() -> Word {
        return registers_[recv];
    };

    auto lhs_two_words = [this, recv]() -> TwoWords {
        return GetTwoRegisters(recv);
    };

    auto lhs_dbl = [&lhs_two_words]() -> Double {
        return ToDbl(lhs_two_words());
    };

    auto rhs_word = [this, src, mod]() -> Word {
        return registers_[src] + mod;
    };

    auto rhs_two_words = [this, src, mod]() -> TwoWords {
        return GetTwoRegisters(src) + static_cast<TwoWords>(mod);
    };

    auto rhs_dbl = [&rhs_two_words]() -> Double {
        return ToDbl(rhs_two_words());
    };

    switch (code) {
        case ADD: {
            registers_[recv] += rhs_word();
            break;
        }

        case SUB: {
            registers_[recv] -= rhs_word();
            break;
        }

        case MUL: {
            TwoWords res = static_cast<TwoWords>(lhs_word()) *
                           static_cast<TwoWords>(rhs_word());
            PutTwoRegisters(res, recv);
            break;
        }

        case DIV: {
            Divide(lhs_two_words(), static_cast<TwoWords>(rhs_word()), recv);
            break;
        }

        case SHL: {
            registers_[recv] <<= rhs_word();
            break;
        }

        case SHR: {
            registers_[recv] >>= rhs_word();
            break;
        }

        case AND: {
            registers_[recv] &= rhs_word();
            break;
        }

        case OR: {
            registers_[recv] |= rhs_word();
            break;
        }

        case XOR: {
            registers_[recv] ^= rhs_word();
            break;
        }

        case MOV: {
            registers_[recv] = rhs_word();
            break;
        }

        case ADDD: {
            PutTwoRegisters(ToUll(lhs_dbl() + rhs_dbl()), recv);
            break;
        }

        case SUBD: {
            PutTwoRegisters(ToUll(lhs_dbl() - rhs_dbl()), recv);
            break;
        }

        case MULD: {
            PutTwoRegisters(ToUll(lhs_dbl() * rhs_dbl()), recv);
            break;
        }

        case DIVD: {
            Double lhs = lhs_dbl();
            Double rhs = rhs_dbl();

            if (rhs == 0) {
                throw ExecutionError::DivisionByZero(lhs, rhs);
            }

            PutTwoRegisters(ToUll(lhs / rhs), recv);
            break;
        }

        case ITOD: {
            PutTwoRegisters(ToUll(static_cast<Double>(rhs_word())), recv);
            break;
        }

        case DTOI: {
            Double dbl = rhs_dbl();
            auto res  = static_cast<TwoWords>(floor(dbl));

            if (res >= static_cast<TwoWords>(kMaxWord)) {
                throw ExecutionError::DtoiOverflow(dbl);
            }

            registers_[recv] = static_cast<Word>(res);
            break;
        }

        case CALL: {
            registers_[recv] = Call(rhs_word());
            break;
        }

        case CMP: {
            WriteComparisonToFlags(lhs_word(), rhs_word());
            break;
        }

        case CMPD: {
            WriteComparisonToFlags(lhs_dbl(), rhs_dbl());
            break;
        }

        case LOADR: {
            registers_[recv] = memory_[rhs_word()];
            break;
        }

        case STORER: {
            memory_[rhs_word()] = registers_[recv];
            break;
        }

        case LOADR2: {
            Address address = rhs_word();

            registers_[recv]     = memory_[address];
            registers_[recv + 1] = memory_[address + 1];
            break;
        }

        case STORER2: {
            Address address = rhs_word();

            memory_[address]     = registers_[recv];
            memory_[address + 1] = registers_[recv + 1];
            break;
        }

        default: {
            throw InternalError::UnknownCommandForFormat(RR, code);
        }
    }

    return true;
}

bool Executor::ExecuteRICommand(CommandCode code, Register reg, Immediate imm) {
    switch (code) {
        case HALT: {
            registers_[reg] = imm;
            return false;
        }

        case SYSCALL: {
            if (!Syscall(reg, static_cast<SyscallCode>(imm))) {
                return false;
            }
            break;
        }

        case ADDI: {
            registers_[reg] += imm;
            break;
        }

        case SUBI: {
            registers_[reg] -= imm;
            break;
        }

        case MULI: {
            auto res = static_cast<TwoWords>(registers_[reg]) * imm;

            PutTwoRegisters(res, reg);
            break;
        }

        case DIVI: {
            Divide(GetTwoRegisters(reg), static_cast<TwoWords>(imm), reg);
            break;
        }

        case LC: {
            registers_[reg] = imm;
            break;
        }

        case SHLI: {
            registers_[reg] <<= imm;
            break;
        }

        case SHRI: {
            registers_[reg] >>= imm;
            break;
        }

        case ANDI: {
            registers_[reg] &= imm;
            break;
        }

        case ORI: {
            registers_[reg] |= imm;
            break;
        }

        case XORI: {
            registers_[reg] ^= imm;
            break;
        }

        case NOT: {
            registers_[reg] = ~registers_[reg];
            break;
        }

        case PUSH: {
            Push(registers_[reg] + imm);
            break;
        }

        case POP: {
            Pop(reg, imm);
            break;
        }

        case CMPI: {
            WriteComparisonToFlags(registers_[reg], imm);
            break;
        }

        default: {
            throw InternalError::UnknownCommandForFormat(RI, code);
        }
    }

    return true;
}

bool Executor::ExecuteJCommand(CommandCode code, Address addr) {
    switch (code) {
        case CALLI: {
            Call(addr);
            break;
        }

        case RET: {
            registers_[kStackRegister] += addr;
            Pop(kInstructionRegister, 0);
            break;
        }

        case JMP: {
            registers_[kInstructionRegister] = addr;
            break;
        }

        case JEQ: {
            Jump(EQUAL, addr);
            break;
        }

        case JNE: {
            Jump(NOT_EQUAL, addr);
            break;
        }

        case JG: {
            Jump(GREATER, addr);
            break;
        }

        case JL: {
            Jump(LESS, addr);
            break;
        }

        case JGE: {
            Jump(GREATER_OR_EQUAL, addr);
            break;
        }

        case JLE: {
            Jump(LESS_OR_EQUAL, addr);
            break;
        }

        default: {
            throw InternalError::UnknownCommandForFormat(J, code);
        }
    }

    return true;
}

bool Executor::ExecuteCommand(Executor::Word command) {
    registers_[kInstructionRegister]++;

    auto code = static_cast<CommandCode>(command >> cmd::kCodeShift);
    if (!kCodeToFormat.contains(code)) {
        throw ExecutionError::UnknownCommand(code);
    }

    switch (CommandFormat format = kCodeToFormat.at(code)) {
        case RM: {
            Register reg = (command >> cmd::kRecvShift) & cmd::kRegisterMask;
            Address addr = command & cmd::kAddressMask;

            return ExecuteRMCommand(code, reg, addr);
        }

        case RR: {
            Receiver recv = (command >> cmd::kRecvShift) & cmd::kRegisterMask;
            Source src    = (command >> cmd::kSrcShift) & cmd::kRegisterMask;
            Modifier mod  = command & cmd::kModMask;

            return ExecuteRRCommand(code, recv, src, mod);
        }

        case RI: {
            Register reg  = (command >> cmd::kRecvShift) & cmd::kRegisterMask;
            Immediate imm = command & cmd::kImmMask;

            return ExecuteRICommand(code, reg, imm);
        }

        case J: {
            Address addr = command & cmd::kAddressMask;

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

    auto read_ui = [&binary]() -> Word {
        Word word{};
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
    size_t code_size   = read_ui();
    size_t consts_size = read_ui();
    size_t data_size   = read_ui();

    // check that the exec file size equals the size specified by the header
    if (exec_size != exec::kHeaderSize + code_size + consts_size + data_size) {
        throw ExecFileError::InvalidExecSize(exec_size,
                                             code_size,
                                             consts_size,
                                             data_size);
    }

    // read the address of the first instruction
    registers_[kInstructionRegister] = read_ui();

    // read the initial stack pointer value
    registers_[kStackRegister] = read_ui();

    // jump to the code segment
    binary.seekg(exec::kCodeSegmentPos);

    // the code_size is denoted in bytes, so we need to divide
    // it by 4 to get the number of machine words
    size_t n_commands = code_size / kWordSize;

    // copy the code to virtual memory
    for (size_t i = 0; i < n_commands; ++i) {
        memory_[i] = read_ui();
    }

    // execute while we are inside the code segment
    // and no command has returned an exit signal
    while (registers_[kInstructionRegister] < n_commands &&
           ExecuteCommand(memory_[registers_[kInstructionRegister]])) {
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
