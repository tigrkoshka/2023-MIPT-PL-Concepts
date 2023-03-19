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

using Word     = uint32_t;
using TwoWords = uint64_t;
using Double   = double;  // TODO: std::float64_t in C++23 (supported by GCC@13)

constexpr size_t kWordSize = sizeof(Word);
constexpr Word kMaxWord    = std::numeric_limits<Word>::max();

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

constexpr size_t kNRegisters = 16;

constexpr Register kCallFrameRegister   = static_cast<Register>(R13);
constexpr Register kStackRegister       = static_cast<Register>(R14);
constexpr Register kInstructionRegister = static_cast<Register>(R15);

extern const std::unordered_map<std::string, Register> kRegisterNameToNum;
extern const std::unordered_map<Register, std::string> kRegisterNumToName;

////////////////////////////////////////////////////////////////////////////////
///                                  Memory                                  ///
////////////////////////////////////////////////////////////////////////////////

using Address = Word;

constexpr size_t kMemorySize = 1ull << 20ull;

}  // namespace karma::detail::specs::arch
