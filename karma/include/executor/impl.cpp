#include "impl.hpp"

#include <exception>  // for exception
#include <iostream>   // for cout, endl
#include <string>     // for string

#include "exec/exec.hpp"
#include "executor/config.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "utils/error.hpp"

namespace karma {

namespace arch = detail::specs::arch;
namespace cmd  = detail::specs::cmd;

Executor::MaybeReturnCode Executor::Impl::ExecuteCmd(cmd::Bin command) {
    storage_->Reg(arch::kInstructionRegister)++;

    auto code = cmd::GetCode(command);
    if (!cmd::kCodeToFormat.contains(code)) {
        throw ExecutionError::UnknownCommand(code);
    }

    switch (cmd::Format format = cmd::kCodeToFormat.at(code)) {
        case cmd::RM: {
            if (!rm_map_.contains(code)) {
                throw InternalError::UnprocessedCommandForFormat(format, code);
            }

            return rm_map_.at(code)(cmd::parse::RM(command));
        }

        case cmd::RR: {
            if (!rr_map_.contains(code)) {
                throw InternalError::UnprocessedCommandForFormat(format, code);
            }

            return rr_map_.at(code)(cmd::parse::RR(command));
        }

        case cmd::RI: {
            if (!ri_map_.contains(code)) {
                throw InternalError::UnprocessedCommandForFormat(format, code);
            }

            return ri_map_.at(code)(cmd::parse::RI(command));
        }

        case cmd::J: {
            if (!j_map_.contains(code)) {
                throw InternalError::UnprocessedCommandForFormat(format, code);
            }

            return j_map_.at(code)(cmd::parse::J(command));
        }

        default: {
            throw InternalError::UnprocessedCommandFormat(format);
        }
    }
}

Executor::ReturnCode Executor::Impl::ExecuteImpl(const std::string& exec,
                                                 const Config& config) {
    Exec::Data data = Exec::Read(exec);

    storage_->PrepareForExecution(data, config);

    while (true) {
        arch::Address curr_address = storage_->Reg(arch::kInstructionRegister);

        if (curr_address >= arch::kMemorySize) {
            throw ExecutionError::ExecPointerOutOfMemory(curr_address);
        }

        if (MaybeReturnCode return_code =
                ExecuteCmd(storage_->Mem(curr_address))) {
            return *return_code;
        }
    }
}

Executor::ReturnCode Executor::Impl::MustExecute(const std::string& exec,
                                                 const Config& config) {
    try {
        return ExecuteImpl(exec, config);
    } catch (const errors::Error& e) {
        throw e;
    } catch (const std::exception& e) {
        throw InternalError::Unexpected(e.what());
    } catch (...) {
        throw InternalError::Unexpected();
    }
}

Executor::ReturnCode Executor::Impl::Execute(const std::string& exec,
                                             const Config& config) {
    try {
        return MustExecute(exec, config);
    } catch (const errors::Error& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
}

}  // namespace karma
