#include "rm_executor.hpp"

#include "executor/executor.hpp"
#include "specs/commands.hpp"

namespace karma {

namespace cmd = detail::specs::cmd;

Executor::RMExecutor::Operation Executor::RMExecutor::LA() {
    return [this](Args args) -> MaybeReturnCode {
        WReg(args.reg) = args.addr;
        return {};
    };
}

Executor::RMExecutor::Operation Executor::RMExecutor::LOAD() {
    return [this](Args args) -> MaybeReturnCode {
        WReg(args.reg) = RMem(args.addr);
        return {};
    };
}

Executor::RMExecutor::Operation Executor::RMExecutor::LOAD2() {
    return [this](Args args) -> MaybeReturnCode {
        WReg(args.reg)     = RMem(args.addr);
        WReg(args.reg + 1) = RMem(args.addr + 1);
        return {};
    };
}

Executor::RMExecutor::Operation Executor::RMExecutor::STORE() {
    return [this](Args args) -> MaybeReturnCode {
        WMem(args.addr) = RReg(args.reg);
        return {};
    };
}

Executor::RMExecutor::Operation Executor::RMExecutor::STORE2() {
    return [this](Args args) -> MaybeReturnCode {
        WMem(args.addr)     = RReg(args.reg);
        WMem(args.addr + 1) = RReg(args.reg + 1);
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
