#include "j_executor.hpp"

#include "executor/return_code.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "specs/flags.hpp"

namespace karma::executor::detail {

namespace arch = karma::detail::specs::arch;

namespace cmd = karma::detail::specs::cmd;

namespace flags = karma::detail::specs::flags;

JExecutor::Operation JExecutor::JMP() {
    return [this](Args args) -> MaybeReturnCode {
        Reg(arch::kInstructionRegister) = args.addr;
        return {};
    };
}

JExecutor::Operation JExecutor::JNE() {
    return [this](Args args) -> MaybeReturnCode {
        Jump(flags::NOT_EQUAL, args.addr);
        return {};
    };
}

JExecutor::Operation JExecutor::JEQ() {
    return [this](Args args) -> MaybeReturnCode {
        Jump(flags::EQUAL, args.addr);
        return {};
    };
}

JExecutor::Operation JExecutor::JLE() {
    return [this](Args args) -> MaybeReturnCode {
        Jump(flags::LESS_OR_EQUAL, args.addr);
        return {};
    };
}

JExecutor::Operation JExecutor::JL() {
    return [this](Args args) -> MaybeReturnCode {
        Jump(flags::LESS, args.addr);
        return {};
    };
}

JExecutor::Operation JExecutor::JGE() {
    return [this](Args args) -> MaybeReturnCode {
        Jump(flags::GREATER_OR_EQUAL, args.addr);
        return {};
    };
}

JExecutor::Operation JExecutor::JG() {
    return [this](Args args) -> MaybeReturnCode {
        Jump(flags::GREATER, args.addr);
        return {};
    };
}

JExecutor::Operation JExecutor::CALLI() {
    return [this](Args args) -> MaybeReturnCode {
        Call(args.addr);
        return {};
    };
}

JExecutor::Operation JExecutor::RET() {
    return [this](Args) -> MaybeReturnCode {
        Return();
        return {};
    };
}

JExecutor::Map JExecutor::GetMap() {
    return {
        {cmd::JMP,   JMP()  },
        {cmd::JNE,   JNE()  },
        {cmd::JEQ,   JEQ()  },
        {cmd::JLE,   JLE()  },
        {cmd::JL,    JL()   },
        {cmd::JGE,   JGE()  },
        {cmd::JG,    JG()   },
        {cmd::CALLI, CALLI()},
        {cmd::RET,   RET()  },
    };
}

}  // namespace karma::executor::detail
