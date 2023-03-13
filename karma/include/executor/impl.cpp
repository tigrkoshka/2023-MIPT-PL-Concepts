#include "impl.hpp"

#include <iostream>  // for cout, endl

#include "errors.hpp"
#include "exec/exec.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "utils/error.hpp"

namespace karma::executor::detail {

using errors::executor::InternalError;
using errors::executor::ExecutionError;
using errors::executor::Error;

namespace arch = karma::detail::specs::arch;

namespace cmd = karma::detail::specs::cmd;

namespace exec = karma::detail::exec;

MaybeReturnCode Impl::ExecuteCommand(cmd::Bin command) {
    storage_->Reg(arch::kInstructionRegister)++;

    auto code = cmd::GetCode(command);
    if (!cmd::kCodeToFormat.contains(code)) {
        throw ExecutionError::UnknownCommand(code);
    }

    switch (cmd::Format format = cmd::kCodeToFormat.at(code)) {
        case cmd::RM: {
            return rm_map_.at(code)(cmd::parse::RM(command));
            break;
        }

        case cmd::RR: {
            return rr_map_.at(code)(cmd::parse::RR(command));
            break;
        }

        case cmd::RI: {
            return ri_map_.at(code)(cmd::parse::RI(command));
            break;
        }

        case cmd::J: {
            return j_map_.at(code)(cmd::parse::J(command));
            break;
        }

        default: {
            throw InternalError::UnprocessedCommandFormat(format);
        }
    }
}

ReturnCode Impl::MustExecute(const std::string& exec_path) {
    exec::Data data = exec::Read(exec_path);

    storage_->PrepareForExecution(data);

    while (true) {
        arch::Address curr_address = storage_->Reg(arch::kInstructionRegister);

        if (curr_address >= arch::kMemorySize) {
            throw ExecutionError::ExecPointerOutOfMemory(curr_address);
        }

        if (MaybeReturnCode return_code =
                ExecuteCommand(storage_->Mem(curr_address))) {
            return *return_code;
        }
    }
}

ReturnCode Impl::Execute(const std::string& exec_path) {
    try {
        return MustExecute(exec_path);
    } catch (const Error& e) {
        std::cout << "executing " << exec_path << ": " << e.what() << std::endl;
        return 1;
    } catch (const errors::Error& e) {
        std::cout << exec_path << ": " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cout << exec_path << ": unexpected exception: " << e.what()
                  << std::endl;
        return 1;
    } catch (...) {
        std::cout << "executing " << exec_path << ": unexpected exception";
        return 1;
    }
}

}  // namespace karma::executor::detail
