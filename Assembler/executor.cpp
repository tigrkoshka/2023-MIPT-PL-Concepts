#include "executor.hpp"

#include <cmath>
#include <concepts>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace karma {

namespace util = detail::util;

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
        requires std::is_same_v<T, Word> || std::is_same_v<T, uint64_t>
    static ExecutionError ImmediateAdditionOverflow(T src_val, T immediate) {
        std::ostringstream ss;
        ss << "a word overflow occurred when adding the immediate operand "
           << immediate << " to the source value " << src_val;
        return ExecutionError{ss.str()};
    }

    template <typename T>
        requires std::is_same_v<T, uint64_t> || std::is_same_v<T, double>
    static ExecutionError DivisionByZero(T dividend, T divisor) {
        std::ostringstream ss;
        ss << "a division by zero occurred when dividing " << dividend << " by "
           << divisor;
        return ExecutionError{ss.str()};
    }

    static ExecutionError QuotientOverflow(uint64_t dividend,
                                           uint64_t divisor) {
        std::ostringstream ss;
        ss << "a quotient overflow occurred when dividing " << dividend
           << " by " << divisor;
        return ExecutionError{ss.str()};
    }

    static ExecutionError DtoiOverflow(double value) {
        std::ostringstream ss;
        ss << "a word overflow occurred when casting " << value
           << " to integer";
        return ExecutionError{ss.str()};
    }

    static ExecutionError InvalidPutCharValue(Word value) {
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

    static ExecFileError TooSmallForHeader(std::streamsize size) {
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

    static ExecFileError InvalidExecSize(std::streamsize exec_size,
                                         uint32_t code_size,
                                         uint32_t consts_size,
                                         uint32_t data_size) {
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

double Executor::ToDbl(uint64_t ull) {
    return *reinterpret_cast<double*>(&ull);
}

uint64_t Executor::ToUll(double dbl) {
    return *reinterpret_cast<uint64_t*>(&dbl);
}

uint64_t Executor::GetTwoRegisters(Register lower) const {
    return (static_cast<uint64_t>(registers_[lower + 1]) << kWordSize) +
           static_cast<uint64_t>(registers_[lower]);
}

void Executor::PutTwoRegisters(uint64_t value, Register lower) {
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

void Executor::Divide(uint64_t lhs, uint64_t rhs, Register recv) {
    if (rhs == 0) {
        throw ExecutionError::DivisionByZero(lhs, rhs);
    }

    uint64_t quotient = lhs / rhs;

    if (quotient > static_cast<uint64_t>(kMaxWord)) {
        throw ExecutionError::QuotientOverflow(lhs, rhs);
    }

    uint64_t remainder = lhs % rhs;

    registers_[recv]     = static_cast<Word>(quotient);
    registers_[recv + 1] = static_cast<Word>(remainder);
}

bool Executor::Syscall(Register reg, SyscallCode code) {
    switch (code) {
        case EXIT: {
            return false;
        }

        case SCANINT: {
            std::cin >> registers_[reg];
            break;
        }

        case SCANDOUBLE: {
            double val{};
            std::cin >> val;

            PutTwoRegisters(ToUll(val), reg);
            break;
        }

        case PRINTINT: {
            std::cout << util::GetSigned(registers_[reg], kWordSize);
            break;
        }

        case PRINTDOUBLE: {
            std::cout << ToDbl(GetTwoRegisters(reg));
            break;
        }

        case GETCHAR: {
            unsigned char val{};
            std::cin >> val;

            registers_[reg] = static_cast<Word>(val);
            break;
        }

        case PUTCHAR: {
            if (registers_[reg] > kMaxChar) {
                throw ExecutionError::InvalidPutCharValue(registers_[reg]);
            }

            std::cout << static_cast<unsigned char>(registers_[reg]);
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

void Executor::Pop(Register recv, Word modifier) {
    registers_[recv] = memory_[++registers_[kStackRegister]] + modifier;
}

Executor::Word Executor::Call(Address callee) {
    Word ret = registers_[kInstructionRegister];

    Push(registers_[kInstructionRegister]);
    registers_[kInstructionRegister] = callee;

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
///                              Execute command                             ///
////////////////////////////////////////////////////////////////////////////////

bool Executor::ExecuteRMCommand(CommandCode command_code,
                                Register reg,
                                Address address) {
    switch (command_code) {
        case LOAD: {
            registers_[reg] = memory_[address];
            break;
        }

        case STORE: {
            memory_[address] = registers_[reg];
            break;
        }

        case LOAD2: {
            registers_[reg]     = memory_[address];
            registers_[reg + 1] = memory_[address + 1];
            break;
        }

        case STORE2: {
            memory_[address]     = registers_[reg];
            memory_[address + 1] = registers_[reg + 1];
            break;
        }

        default: {
            throw InternalError::UnknownCommandForFormat(RM, command_code);
        }
    }

    return true;
}

bool Executor::ExecuteRRCommand(CommandCode command_code,
                                Register recv,
                                Register src,
                                int32_t modifier) {
    auto lhs_32 = [this, recv]() -> Word {
        return registers_[recv];
    };

    auto lhs_64 = [this, recv]() -> uint64_t {
        return GetTwoRegisters(recv);
    };

    auto lhs_dbl = [&lhs_64]() -> double {
        return ToDbl(lhs_64());
    };

    auto rhs_32 = [this, src, modifier]() -> Word {
        Word val = registers_[src];

        if (val > kMaxWord - modifier) {
            throw ExecutionError::ImmediateAdditionOverflow(val, modifier);
        }

        return val + modifier;
    };

    auto rhs_64 = [this, src, modifier]() -> uint64_t {
        uint64_t val    = GetTwoRegisters(src);
        auto modifier64 = static_cast<uint64_t>(modifier);

        if (val > kMaxTwoWords - modifier64) {
            throw ExecutionError::ImmediateAdditionOverflow(val, modifier64);
        }

        return val + modifier64;
    };

    auto rhs_dbl = [&rhs_64]() -> double {
        return ToDbl(rhs_64());
    };

    switch (command_code) {
        case ADD: {
            registers_[recv] += rhs_32();
            break;
        }

        case SUB: {
            registers_[recv] -= rhs_32();
            break;
        }

        case MUL: {
            uint64_t res = static_cast<uint64_t>(lhs_32()) *
                           static_cast<uint64_t>(rhs_32());
            PutTwoRegisters(res, recv);
            break;
        }

        case DIV: {
            Divide(lhs_64(), static_cast<uint64_t>(rhs_32()), recv);
            break;
        }

        case SHL: {
            registers_[recv] <<= rhs_32();
            break;
        }

        case SHR: {
            registers_[recv] >>= rhs_32();
            break;
        }

        case AND: {
            registers_[recv] &= rhs_32();
            break;
        }

        case OR: {
            registers_[recv] |= rhs_32();
            break;
        }

        case XOR: {
            registers_[recv] ^= rhs_32();
            break;
        }

        case MOV: {
            registers_[recv] = rhs_32();
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
            double lhs = lhs_dbl();
            double rhs = rhs_dbl();

            if (rhs == 0) {
                throw ExecutionError::DivisionByZero(lhs, rhs);
            }

            PutTwoRegisters(ToUll(lhs / rhs), recv);
            break;
        }

        case ITOD: {
            PutTwoRegisters(ToUll(static_cast<double>(rhs_32())), recv);
            break;
        }

        case DTOI: {
            double dbl = rhs_dbl();
            auto res   = static_cast<uint64_t>(floor(dbl));

            if (res >= static_cast<uint64_t>(kMaxWord)) {
                throw ExecutionError::DtoiOverflow(dbl);
            }

            registers_[recv] = static_cast<Word>(res);
            break;
        }

        case CALL: {
            registers_[recv] = Call(rhs_32());
            break;
        }

        case CMP: {
            WriteComparisonToFlags(lhs_32(), rhs_32());
            break;
        }

        case CMPD: {
            WriteComparisonToFlags(lhs_dbl(), rhs_dbl());
            break;
        }

        case LOADR: {
            registers_[recv] = memory_[rhs_32()];
            break;
        }

        case STORER: {
            memory_[rhs_32()] = registers_[recv];
            break;
        }

        case LOADR2: {
            Address address = rhs_32();

            registers_[recv]     = memory_[address];
            registers_[recv + 1] = memory_[address + 1];
            break;
        }

        case STORER2: {
            Address address = rhs_32();

            memory_[address]     = registers_[recv];
            memory_[address + 1] = registers_[recv + 1];
            break;
        }

        default: {
            throw InternalError::UnknownCommandForFormat(RR, command_code);
        }
    }

    return true;
}

bool Executor::ExecuteRICommand(CommandCode command_code,
                                Register reg,
                                int32_t immediate) {
    switch (command_code) {
        case HALT: {
            registers_[reg] = immediate;
            return false;
        }

        case SYSCALL: {
            if (!Syscall(reg, static_cast<SyscallCode>(immediate))) {
                return false;
            }
            break;
        }

        case ADDI: {
            registers_[reg] += immediate;
            break;
        }

        case SUBI: {
            registers_[reg] -= immediate;
            break;
        }

        case MULI: {
            uint64_t res = static_cast<uint64_t>(registers_[reg]) * immediate;

            PutTwoRegisters(res, reg);
            break;
        }

        case DIVI: {
            Divide(GetTwoRegisters(reg), static_cast<uint64_t>(immediate), reg);
            break;
        }

        case LC: {
            registers_[reg] = immediate;
            break;
        }

        case SHLI: {
            registers_[reg] <<= immediate;
            break;
        }

        case SHRI: {
            registers_[reg] >>= immediate;
            break;
        }

        case ANDI: {
            registers_[reg] &= immediate;
            break;
        }

        case ORI: {
            registers_[reg] |= immediate;
            break;
        }

        case XORI: {
            registers_[reg] ^= immediate;
            break;
        }

        case NOT: {
            registers_[reg] = ~registers_[reg];
            break;
        }

        case PUSH: {
            Push(registers_[reg] + immediate);
            break;
        }

        case POP: {
            Pop(reg, immediate);
            break;
        }

        case CMPI: {
            WriteComparisonToFlags(registers_[reg], immediate);
            break;
        }

        default: {
            throw InternalError::UnknownCommandForFormat(RI, command_code);
        }
    }

    return true;
}

bool Executor::ExecuteJCommand(CommandCode command_code, Address addr) {
    switch (command_code) {
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
            throw InternalError::UnknownCommandForFormat(J, command_code);
        }
    }

    return true;
}

bool Executor::ExecuteCommand(Executor::Word command) {
    registers_[kInstructionRegister]++;

    auto command_code = static_cast<CommandCode>(command >> cmd::kCodeShift);
    if (!kCodeToFormat.contains(command_code)) {
        throw ExecutionError::UnknownCommand(command_code);
    }

    switch (CommandFormat format = kCodeToFormat.at(command_code)) {
        case RM: {
            Register reg = (command >> cmd::kRecvShift) & cmd::kRegisterMask;
            Address addr = command & cmd::kAddressMask;

            return ExecuteRMCommand(command_code, reg, addr);
        }

        case RR: {
            Register recv = (command >> cmd::kRecvShift) & cmd::kRegisterMask;
            Register src  = (command >> cmd::kSrcShift) & cmd::kRegisterMask;
            int32_t modifier =
                util::GetSigned(command & cmd::kModMask, cmd::kModSize);

            return ExecuteRRCommand(command_code, recv, src, modifier);
        }

        case RI: {
            Register reg = (command >> cmd::kRecvShift) & cmd::kRegisterMask;
            int32_t immediate =
                util::GetSigned(command & cmd::kImmMask, cmd::kImmSize);

            return ExecuteRICommand(command_code, reg, immediate);
        }

        case J: {
            Word addr = command & cmd::kAddressMask;

            return ExecuteJCommand(command_code, addr);
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
    std::streamsize exec_size = binary.tellg();
    if (exec_size < exec::kHeaderSize) {
        throw ExecFileError::TooSmallForHeader(exec_size);
    }

    // reset input position indicator
    binary.seekg(0);

    auto read_ui = [&binary]() -> uint32_t {
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
    uint32_t code_size   = read_ui();
    uint32_t consts_size = read_ui();
    uint32_t data_size   = read_ui();

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
    uint32_t n_commands = code_size / kWordSize;

    // copy the code to virtual memory
    for (uint32_t i = 0; i < n_commands; ++i) {
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
