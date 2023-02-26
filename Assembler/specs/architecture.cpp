#include "architecture.hpp"

#include <string>
#include <unordered_map>

#include "../utils.hpp"

namespace karma::detail::specs::arch {

const std::unordered_map<std::string, Register> kRegisterToNum = {
    {"r0",  R0 },
    {"r1",  R1 },
    {"r2",  R2 },
    {"r3",  R3 },
    {"r4",  R4 },
    {"r5",  R5 },
    {"r6",  R6 },
    {"r7",  R7 },
    {"r8",  R8 },
    {"r9",  R9 },
    {"r10", R10},
    {"r11", R11},
    {"r12", R12},
    {"r13", R13},
    {"r14", R14},
    {"r15", R15},
};

const std::unordered_map<Register, std::string> kRegisterToString =
    util::RevertMap(kRegisterToNum);

}  // namespace karma::detail::specs::arch
