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
    R0  = 0,
    R1  = 1,
    R2  = 2,
    R3  = 3,
    R4  = 4,
    R5  = 5,
    R6  = 6,
    R7  = 7,
    R8  = 8,
    R9  = 9,
    R10 = 10,
    R11 = 11,
    R12 = 12,
    R13 = 13,
    R14 = 14,
    R15 = 15,
};

static const Register kCallFrameRegister   = R13;
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