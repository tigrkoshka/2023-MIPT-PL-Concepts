#include "rm_executor.hpp"

#include "executor/executor.hpp"
#include "specs/commands.hpp"

namespace karma {

namespace cmd = karma::detail::specs::cmd;

Executor::RMExecutor::Operation Executor::RMExecutor::LA() {
    return [this](Args args) -> MaybeReturnCode {
        Reg(args.reg) = args.addr;
        return {};
    };
}

Executor::RMExecutor::Operation Executor::RMExecutor::LOAD() {
    return [this](Args args) -> MaybeReturnCode {
        Reg(args.reg) = Mem(args.addr);
        return {};
    };
}

Executor::RMExecutor::Operation Executor::RMExecutor::LOAD2() {
    return [this](Args args) -> MaybeReturnCode {
        Reg(args.reg)     = Mem(args.addr);
        Reg(args.reg + 1) = Mem(args.addr + 1);
        return {};
    };
}

Executor::RMExecutor::Operation Executor::RMExecutor::STORE() {
    return [this](Args args) -> MaybeReturnCode {
        Mem(args.addr) = Reg(args.reg);
        return {};
    };
}

Executor::RMExecutor::Operation Executor::RMExecutor::STORE2() {
    return [this](Args args) -> MaybeReturnCode {
        Mem(args.addr)     = Reg(args.reg);
        Mem(args.addr + 1) = Reg(args.reg + 1);
        return {};
    };
}

Executor::RMExecutor::Map Executor::RMExecutor::GetMap() {
    return {
        {cmd::LA,     LA()    },
        {cmd::LOAD,   LOAD()  },
        {cmd::LOAD2,  LOAD2() },
        {cmd::STORE,  STORE() },
        {cmd::STORE2, STORE2()},
    };
}

}  // namespace karma
