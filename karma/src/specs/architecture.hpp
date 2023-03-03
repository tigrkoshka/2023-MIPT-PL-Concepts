#pragma once

#include <cstddef>        // for size_t
#include <cstdint>        // for uint32_t, uint64_t
#include <limits>         // for numeric_limits
#include <string>         // for string
#include <unordered_map>  // for unordered_map

namespace karma::detail::specs::arch {

////////////////////////////////////////////////////////////////////////////////
///                        Architecture-defined types                        ///
////////////////////////////////////////////////////////////////////////////////

inline namespace types {

using Word     = uint32_t;
using TwoWords = uint64_t;
using Double   = double;  // TODO: std::float64_t in C++23 (supported by GCC@13)
using Char     = unsigned char;

const size_t kWordSize     = sizeof(Word);
const Word kMaxWord        = std::numeric_limits<Word>::max();
static const Char kMaxChar = std::numeric_limits<Char>::max();

}  // namespace types

////////////////////////////////////////////////////////////////////////////////
///                                 Registers                                ///
////////////////////////////////////////////////////////////////////////////////

using Register = Word;
enum RegisterEnum : Register {
    R0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
};

static const Register kStackRegister       = R14;
static const Register kInstructionRegister = R15;

extern const std::unordered_map<std::string, Register> kRegisterNameToNum;
extern const std::unordered_map<Register, std::string> kRegisterNumToName;

static const size_t kNRegisters = 16ull;

////////////////////////////////////////////////////////////////////////////////
///                                  Memory                                  ///
////////////////////////////////////////////////////////////////////////////////

using Address                   = Word;
static const size_t kMemorySize = 1ull << 20ull;

}  // namespace karma::detail::specs::arch