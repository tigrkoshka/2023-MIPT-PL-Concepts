#include "rm_executor.hpp"

#include "executor/return_code.hpp"
#include "specs/commands.hpp"

namespace karma::executor::detail {

namespace cmd = karma::detail::specs::cmd;

RMExecutor::Operation RMExecutor::LA() {
    return [this](Args args) -> MaybeReturnCode {
        Reg(args.reg) = args.addr;
        return {};
    };
}

RMExecutor::Operation RMExecutor::LOAD() {
    return [this](Args args) -> MaybeReturnCode {
        Reg(args.reg) = Mem(args.addr);
        return {};
    };
}

RMExecutor::Operation RMExecutor::LOAD2() {
    return [this](Args args) -> MaybeReturnCode {
        Reg(args.reg)     = Mem(args.addr);
        Reg(args.reg + 1) = Mem(args.addr + 1);
        return {};
    };
}

RMExecutor::Operation RMExecutor::STORE() {
    return [this](Args args) -> MaybeReturnCode {
        Mem(args.addr) = Reg(args.reg);
        return {};
    };
}

RMExecutor::Operation RMExecutor::STORE2() {
    return [this](Args args) -> MaybeReturnCode {
        Mem(args.addr)     = Reg(args.reg);
        Mem(args.addr + 1) = Reg(args.reg + 1);
        return {};
    };
}

RMExecutor::Map RMExecutor::GetMap() {
    return {
        {cmd::LA,     LA()    },
        {cmd::LOAD,   LOAD()  },
        {cmd::LOAD2,  LOAD2() },
        {cmd::STORE,  STORE() },
        {cmd::STORE2, STORE2()},
    };
}

}  // namespace karma::executor::detail
