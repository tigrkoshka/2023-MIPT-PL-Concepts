#include "executor.hpp"

#include <fstream>

namespace karma {

using namespace detail; // NOLINT(google-build-using-namespace)

////////////////////////////////////////////////////////////////////////////////
///                                Errors                                    ///
////////////////////////////////////////////////////////////////////////////////

std::string Executor::Errors::UnknownFormat(CommandFormat format) {
    std::ostringstream ss;
    ss << "unknown command format " << format;
    return ss.str();
}

std::string Executor::Errors::UnknownCommand(CommandFormat format,
                                             CommandCode code) {
    std::ostringstream ss;
    ss << "unknown command code " << code << " for command format \""
       << kFormatToString.at(format) << "\"";
    return ss.str();
}

std::string Executor::Errors::UnknownSyscallCode(SyscallCode code) {
    std::ostringstream ss;
    ss << "unknown syscall code" << code;
    return ss.str();
}

template <typename T>
    requires std::is_same_v<T, Executor::Word> || std::is_same_v<T, uint64_t>
std::string Executor::Errors::ImmediateAdditionOverflow(T src_val,
                                                        T immediate) {
    std::ostringstream ss;
    ss << "a word overflow occurred when adding the immediate operand "
       << immediate << " to the source value " << src_val;
    return ss.str();
}

template <typename T>
    requires std::is_same_v<T, uint64_t> || std::is_same_v<T, double>
std::string Executor::Errors::DivisionByZero(T dividend, T divisor) {
    std::ostringstream ss;
    ss << "a division by zero occurred when dividing " << dividend << " by "
       << divisor;
    return ss.str();
}

std::string Executor::Errors::QuotientOverflow(uint64_t dividend,
                                               uint64_t divisor) {
    std::ostringstream ss;
    ss << "a quotient overflow occurred when dividing " << dividend << " by "
       << divisor;
    return ss.str();
}

std::string Executor::Errors::DtoiOverflow(double value) {
    std::ostringstream ss;
    ss << "a word overflow occurred when casting " << value << " to integer";
    return ss.str();
}

std::string Executor::Errors::InvalidPutCharValue(Word value) {
    std::ostringstream ss;
    ss << "the value in the register for the PUTCHAR syscall is " << value
       << ", which is an invalid char value, because it is greater than 255";
    return ss.str();
}

////////////////////////////////////////////////////////////////////////////////
///                                 Utils                                    ///
////////////////////////////////////////////////////////////////////////////////

double Executor::ToDbl(uint64_t ull) {
    union {
        uint64_t ull;
        double dbl;
    } ull_dbl{.ull = ull};

    return ull_dbl.dbl;
}

uint64_t Executor::ToUll(double dbl) {
    union {
        uint64_t ull;
        double dbl;
    } ull_dbl{.dbl = dbl};

    return ull_dbl.ull;
}

uint64_t Executor::GetTwoRegisters(Register lower) const {
    return (static_cast<uint64_t>(registers_[lower + 1]) << kWordSize) +
           static_cast<uint64_t>(registers_[lower]);
}

void Executor::PutTwoRegisters(uint64_t value, Register lower) {
    registers_[lower]     = static_cast<Word>(value);
    registers_[lower + 1] = static_cast<Word>(value >> kWordSize);
}

template <typename T>
    requires std::totally_ordered<T>
void Executor::WriteComparisonToFlags(T lhs, T rhs) {
    auto cmp_res = lhs <=> rhs;

    if (cmp_res == nullptr) {
        flags_ = kEqual;
    } else if (cmp_res > nullptr) {
        flags_ = kGreater;
    } else {
        // condition "cmp_res < 0" is true
        // because lhs and rhs are comparable
        // because of the template type constraint
        flags_ = kLess;
    }
}

void Executor::Jump(Condition condition, Address dst) {
    if ((flags_ & condition) != 0) {
        registers_[kInstructionRegister] = dst;
    }
}

////////////////////////////////////////////////////////////////////////////////
///                            Separate commands                             ///
////////////////////////////////////////////////////////////////////////////////

void Executor::Divide(uint64_t lhs, uint64_t rhs, Register recv) {
    if (rhs == 0) {
        throw std::runtime_error(Errors::DivisionByZero(lhs, rhs));
    }

    uint64_t quotient = lhs / rhs;

    if (quotient > static_cast<uint64_t>(kMaxWord)) {
        throw std::runtime_error(Errors::QuotientOverflow(lhs, rhs));
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
            union {
                uint64_t ull;
                double dbl;
            } ull_dbl;
            std::cin >> ull_dbl.dbl;

            PutTwoRegisters(ull_dbl.ull, reg);
            break;
        }

        case PRINTINT: {
            std::cout << utils::GetSignedInt(registers_[reg], 32u);
            break;
        }

        case PRINTDOUBLE: {
            std::cout << ToDbl(GetTwoRegisters(reg));
            break;
        }

        case GETCHAR: {
            uint8_t c;
            std::cin >> c;

            registers_[reg] = static_cast<Word>(c);
            break;
        }

        case PUTCHAR: {
            if (registers_[reg] > kMaxChar) {
                throw std::runtime_error(
                    Errors::InvalidPutCharValue(registers_[reg]));
            }

            std::cout << static_cast<uint8_t>(registers_[reg]);
            break;
        }

        default: {
            throw std::runtime_error(Errors::UnknownSyscallCode(code));
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
///                             Execute command                              ///
////////////////////////////////////////////////////////////////////////////////

bool Executor::ExecuteRMCommand(CommandCode command_code,
                                Register reg,
                                Address mem) {
    switch (command_code) {
        case LOAD: {
            registers_[reg] = memory_[mem];
            break;
        }

        case STORE: {
            memory_[mem] = registers_[reg];
            break;
        }

        case LOAD2: {
            registers_[reg]     = memory_[mem];
            registers_[reg + 1] = memory_[mem + 1];
            break;
        }

        case STORE2: {
            memory_[mem]     = registers_[reg];
            memory_[mem + 1] = registers_[reg + 1];
            break;
        }

        default: {
            throw std::runtime_error(Errors::UnknownCommand(RM, command_code));
        }
    }

    return true;
}

bool Executor::ExecuteRRCommand(CommandCode command_code,
                                Register recv,
                                Register src,
                                Word immediate) {
    auto lhs_32 = [this, recv]() -> Word {
        return registers_[recv];
    };

    auto lhs_64 = [this, recv]() -> uint64_t {
        return GetTwoRegisters(recv);
    };

    auto lhs_dbl = [&lhs_64]() -> double {
        return ToDbl(lhs_64());
    };

    auto rhs_32 = [this, src, immediate]() -> Word {
        Word val = registers_[src];

        if (val > kMaxWord - immediate) {
            throw std::runtime_error(
                Errors::ImmediateAdditionOverflow(val, immediate));
        }

        return val + immediate;
    };

    auto rhs_64 = [this, src, immediate]() -> uint64_t {
        uint64_t val         = GetTwoRegisters(src);
        uint64_t immediate64 = static_cast<uint64_t>(immediate);

        if (val > kMaxTwoWords - immediate64) {
            throw std::runtime_error(
                Errors::ImmediateAdditionOverflow(val, immediate64));
        }

        return val + immediate64;
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
            double lhs = lhs_dbl(), rhs = rhs_dbl();
            if (rhs == 0) {
                throw std::runtime_error(Errors::DivisionByZero(lhs, rhs));
            }

            PutTwoRegisters(ToUll(lhs / rhs), recv);
            break;
        }

        case ITOD: {
            PutTwoRegisters(ToUll(static_cast<double>(rhs_32())), recv);
            break;
        }

        case DTOI: {
            double dbl   = rhs_dbl();
            uint64_t res = static_cast<uint64_t>(floor(dbl));

            if (res >= static_cast<uint64_t>(kMaxWord)) {
                throw std::runtime_error(Errors::DtoiOverflow(dbl));
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
            throw std::runtime_error(Errors::UnknownCommand(RR, command_code));
        }
    }

    return true;
}

bool Executor::ExecuteRICommand(CommandCode command_code,
                                Register reg,
                                Word immediate) {
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
            throw std::runtime_error(Errors::UnknownCommand(RI, command_code));
        }
    }

    return true;
}

bool Executor::ExecuteJCommand(CommandCode command_code, Word immediate) {
    switch (command_code) {
        case CALLI: {
            Call(immediate);
            break;
        }

        case RET: {
            registers_[kStackRegister] += immediate;
            Pop(kInstructionRegister, 0);
            break;
        }

        case JMP: {
            registers_[kInstructionRegister] = immediate;
            break;
        }

        case JEQ: {
            Jump(EQUAL, immediate);
            break;
        }

        case JNE: {
            Jump(NOT_EQUAL, immediate);
            break;
        }

        case JG: {
            Jump(GREATER, immediate);
            break;
        }

        case JL: {
            Jump(LESS, immediate);
            break;
        }

        case JGE: {
            Jump(GREATER_OR_EQUAL, immediate);
            break;
        }

        case JLE: {
            Jump(LESS_OR_EQUAL, immediate);
            break;
        }

        default: {
            throw std::runtime_error(Errors::UnknownCommand(J, command_code));
        }
    }

    return true;
}

bool Executor::ExecuteCommand(Executor::Word command) {
    auto command_code = static_cast<CommandCode>(command >> 24u);

    registers_[kInstructionRegister]++;

    switch (CommandFormat format = kCodeToFormat.at(command_code)) {
        case RM: {
            Register reg = (command >> 20u) & 0b1111u;
            Address mem  = (command << 12u) >> 12u;

            return ExecuteRMCommand(command_code, reg, mem);
        }

        case RR: {
            Register recv  = (command >> 20u) & 0b1111u;
            Register src   = (command >> 16u) & 0b1111u;
            Word immediate = utils::GetSignedInt((command << 16u) >> 16u, 16u);

            return ExecuteRRCommand(command_code, recv, src, immediate);
        }

        case RI: {
            Register reg   = (command >> 20u) & 15u;
            Word immediate = utils::GetSignedInt((command << 12u) >> 12u, 20u);

            return ExecuteRICommand(command_code, reg, immediate);
        }

        case J: {
            Word immediate = (command << 12u) >> 12u;

            return ExecuteJCommand(command_code, immediate);
        }

        default: {
            throw std::runtime_error(Errors::UnknownFormat(format));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
///                              Execute binary                              ///
////////////////////////////////////////////////////////////////////////////////

void Executor::ExecuteImpl(const std::string& exec_path) {
    std::ifstream binary(exec_path, std::ios::in | std::ios::binary);
    char* bin_buff = new char[4];

    auto read_ui = [&binary, &bin_buff]() -> uint32_t {
        binary.read(bin_buff, 4);
        return *reinterpret_cast<uint32_t*>(bin_buff);
    };

    // skip the introductory string
    binary.seekg(16);

    // read the code size (which is written in bytes, so we need
    // to divide it by 4 to get the number of machine words)
    uint32_t command_num = read_ui() >> 2u;

    // skip the constants and data sections sizes for now
    // TODO: introduce constants
    binary.seekg(28);

    // read the address of the first instruction
    registers_[kInstructionRegister] = read_ui();

    // read the initial stack pointer value
    registers_[kStackRegister] = read_ui();

    // jump to the code segment
    binary.seekg(512);

    // copy the code to virtual memory
    for (uint32_t i = 0; i < command_num; ++i) {
        memory_[i] = read_ui();
    }

    // execute while we are inside the code segment
    // and no command has returned an exit signal
    while (registers_[kInstructionRegister] < command_num &&
           ExecuteCommand(memory_[registers_[kInstructionRegister]])) {
    }
}

void Executor::Execute(const std::string& exec_path) {
    try {
        ExecuteImpl(exec_path);
    } catch (const std::overflow_error& e) {
        std::cout << "Overflow while executing binary \"" << exec_path
                  << "\": " << e.what() << std::endl;
    } catch (const std::runtime_error& e) {
        std::cout << "Execution error in binary \"" << exec_path
                  << "\": " << e.what() << std::endl;
    }  catch (const std::exception& e) {
        std::cout << "Exception while executing binary \"" << exec_path
                  << "\": " << e.what() << std::endl;
    } catch (...) {
        std::cout << "Unexpected exception while executing binary \""
                  << exec_path << "\"" << std::endl;
    }
}

}  // namespace karma
