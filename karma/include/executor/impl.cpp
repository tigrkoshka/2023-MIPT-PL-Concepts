#include "impl.hpp"

#include <exception>  // for exception
#include <iostream>   // for cerr, endl
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
    auto code = cmd::GetCode(command);
    if (!cmd::kCodeToFormat.contains(code)) {
        throw ExecutionError::UnknownCommand(code);
    }

    switch (cmd::Format format = cmd::kCodeToFormat.at(code)) {
        case cmd::RM: {
            if (!rm_map_.contains(code)) {
                throw InternalError::UnprocessedCommandForFormat(format, code);
            }

            auto args = cmd::parse::RM(command);
            return rm_map_.at(code)(args);
        }

        case cmd::RR: {
            if (!rr_map_.contains(code)) {
                throw InternalError::UnprocessedCommandForFormat(format, code);
            }

            auto args = cmd::parse::RR(command);
            return rr_map_.at(code)(args);
        }

        case cmd::RI: {
            if (!ri_map_.contains(code)) {
                throw InternalError::UnprocessedCommandForFormat(format, code);
            }

            auto args = cmd::parse::RI(command);
            return ri_map_.at(code)(args);
        }

        case cmd::J: {
            if (!j_map_.contains(code)) {
                throw InternalError::UnprocessedCommandForFormat(format, code);
            }

            auto args = cmd::parse::J(command);
            return j_map_.at(code)(args);
        }

        default: {
            throw InternalError::UnprocessedCommandFormat(format);
        }
    }
}

Executor::ReturnCode Executor::Impl::ExecuteImpl(const std::string& exec_path,
                                                 const Config& config) {
    Exec::Data data = Exec::Read(exec_path);

    storage_->PrepareForExecution(data, config);

    while (true) {
        arch::Address curr_address =
            storage_->RReg(arch::kInstructionRegister, true);

        if (curr_address >= arch::kMemorySize) {
            throw ExecutionError::ExecPointerOutOfMemory(curr_address);
        }

        storage_->WReg(arch::kInstructionRegister, true)++;

        // do not check if the current command address is out of the initial
        // code segment to allow for programs to write and execute the code
        // at runtime (bad practice, but should be allowed in assembler)

        if (MaybeReturnCode return_code =
                ExecuteCmd(storage_->RMem(curr_address, true))) {
            return *return_code;
        }
    }
}

Executor::ReturnCode Executor::Impl::MustExecute(const std::string& exec_path,
                                                 const Config& config) {
    using std::string_literals::operator""s;

    try {
        return ExecuteImpl(exec_path, config);
    } catch (const errors::Error& e) {
        throw e;
    } catch (const std::exception& e) {
        throw errors::executor::Error("unexpected executor exception: "s +
                                      e.what());
    } catch (...) {
        throw errors::executor::Error("unexpected executor exception");
    }
}

Executor::ReturnCode Executor::Impl::Execute(const std::string& exec_path,
                                             const Config& config) {
    try {
        return MustExecute(exec_path, config);
    } catch (const errors::Error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

}  // namespace karma
