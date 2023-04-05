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

    switch (const cmd::Format format = cmd::kCodeToFormat.at(code)) {
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
                                                 const Config& config,
                                                 std::ostream& log) {
    log << "[executor]: reading the executable file" << std::endl;

    const Exec::Data data = Exec::Read(exec_path);

    log << "[executor]: successfully read the executable file" << std::endl;

    log << "[executor]: preparing for execution" << std::endl;

    storage_->PrepareForExecution(data, config, log);

    log << "[executor]: successfully prepared for execution" << std::endl;

    log << "[executor]: executing the program" << std::endl;

    while (true) {
        const arch::Address curr_address =
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
            log << "[executor]: the program finished execution with code "
                << *return_code << std::endl;
            return *return_code;
        }
    }
}

Executor::ReturnCode Executor::Impl::MustExecute(const std::string& exec_path,
                                                 const Config& config,
                                                 std::ostream& log) {
    using std::string_literals::operator""s;

    try {
        return ExecuteImpl(exec_path, config, log);
    } catch (const errors::executor::Error& e) {
        log << "[executor]: error: " << e.what() << std::endl;
        throw e;
    } catch (const errors::Error& e) {
        log << "[executor]: error: " << e.what() << std::endl;
        throw errors::executor::Error(
            "error during execution process "
            "(not directly related to the execution itself): "s +
            e.what());
    } catch (const std::exception& e) {
        log << "[executor]: unexpected exception: " << e.what() << std::endl;
        throw errors::executor::Error("unexpected exception in executor: "s +
                                      e.what());
    } catch (...) {
        log << "[executor]: unexpected exception" << std::endl;
        throw errors::executor::Error(
            "unexpected exception in executor "
            "(no additional info can be provided)");
    }
}

Executor::ReturnCode Executor::Impl::Execute(const std::string& exec_path,
                                             const Config& config,
                                             std::ostream& log) {
    try {
        return MustExecute(exec_path, config, log);
    } catch (const errors::Error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

}  // namespace karma
