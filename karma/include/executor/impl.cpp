#include "impl.hpp"

#include <exception>  // for exception
#include <iostream>   // for cout, endl
#include <string>     // for string

#include "exec/exec.hpp"
#include "executor/config.hpp"
#include "executor/errors.hpp"
#include "executor/return_code.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "utils/error.hpp"

namespace karma {

using errors::executor::InternalError;
using errors::executor::ExecutionError;
using errors::executor::Error;

namespace arch = karma::detail::specs::arch;

namespace cmd = karma::detail::specs::cmd;

namespace exec = karma::detail::exec;

Executor::Impl::MaybeReturnCode Executor::Impl::ExecuteCmd(cmd::Bin command) {
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

Executor::Impl::ReturnCode Executor::Impl::MustExecute(const std::string& exec,
                                                       const Config& config) {
    exec::Data data = exec::Read(exec);

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

Executor::Impl::ReturnCode Executor::Impl::Execute(const std::string& exec,
                                                   const Config& config) {
    try {
        return MustExecute(exec, config);
    } catch (const Error& e) {
        std::cout << "executing " << exec << ": " << e.what() << std::endl;
        return 1;
    } catch (const errors::Error& e) {
        std::cout << exec << ": " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cout << exec << ": unexpected exception: " << e.what()
                  << std::endl;
        return 1;
    } catch (...) {
        std::cout << "executing " << exec << ": unexpected exception";
        return 1;
    }
}

}  // namespace karma
