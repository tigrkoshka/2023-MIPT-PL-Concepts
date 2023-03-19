#include "impl.hpp"

#include <cstddef>     // for size_t
#include <exception>   // for exception
#include <filesystem>  // for path
#include <iostream>    // for ostream, cerr
#include <optional>    // for optional
#include <string>      // for string
#include <utility>     // for move
#include <vector>      // for vector

#include "compiler/compiler.hpp"
#include "compiler/exec_data.hpp"
#include "compiler/file_compiler.hpp"
#include "compiler/includes.hpp"
#include "exec/exec.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "specs/exec.hpp"
#include "utils/error.hpp"
#include "utils/vector.hpp"

namespace karma {

namespace utils = detail::utils;
namespace arch  = detail::specs::arch;
namespace args  = detail::specs::cmd::args;
namespace exec  = detail::specs::exec;

////////////////////////////////////////////////////////////////////////////////
///                            Labels substitution                           ///
////////////////////////////////////////////////////////////////////////////////

void Compiler::Impl::FillLabels(const FilesDataMap& files_data,
                                std::shared_ptr<Labels>& labels,
                                std::shared_ptr<Entrypoint>& entrypoint) {
    for (const auto& [label, usages] : labels->GetUsages()) {
        std::optional<size_t> definition_opt = labels->TryGetDefinition(label);
        if (!definition_opt) {
            throw CompileError::UndefinedLabel(
                {label, labels->GetUsageSample(label)});
        }

        auto definition = static_cast<args::Address>(*definition_opt);
        for (auto [file, file_usages] : usages) {
            for (auto n_cmd : file_usages) {
                // the address always occupies the last
                // bits of the command binary
                files_data.at(file)->Code()[n_cmd] |= definition;
            }
        }
    }

    if (std::optional<std::string> entry = labels->TryGetEntrypointLabel()) {
        std::optional<size_t> definition = labels->TryGetDefinition(*entry);
        if (!definition) {
            std::optional<std::string> entry_pos = entrypoint->TryGetPos();
            if (!entry_pos) {
                // this is an internal error, because we have checked
                // the presence of address in the Entrypoint on an earlier
                // stage in the PrepareExecData method
                throw InternalError::NoEntrypointPosInLabelSubstitution();
            }

            throw CompileError::UndefinedLabel({*entry, *entry_pos});
        }

        entrypoint->SetAddress(static_cast<args::Address>(*definition));
    }
}

////////////////////////////////////////////////////////////////////////////////
///                                Prepare data                              ///
////////////////////////////////////////////////////////////////////////////////

Exec::Data Compiler::Impl::PrepareExecData(const Files& files,
                                           std::ostream& log) {
    // TODO: multithreading
    std::shared_ptr<Labels> labels         = std::make_shared<Labels>();
    std::shared_ptr<Entrypoint> entrypoint = std::make_shared<Entrypoint>();

    std::vector<ExecData> files_data(files.size());
    FilesDataMap files_data_map;
    size_t code_size      = 0;
    size_t constants_size = 0;
    for (size_t i = 0; i < files.size(); ++i) {
        log << "[compiler]: compiling " << files[i]->Path() << std::endl;

        FileCompiler file_compiler{files[i],
                                   labels,
                                   entrypoint,
                                   code_size,
                                   constants_size};

        log << "[compiler]: successfully compiled " << files[i]->Path()
            << std::endl;

        files_data[i]                  = std::move(file_compiler).PrepareData();
        files_data_map[files[i].get()] = &files_data[i];

        code_size += files_data[i].Code().size();
        constants_size += files_data[i].Constants().size();
    }

    // even if the entrypoint was defines via a label,
    // the address should be recorded as 0 rather than std::nullopt
    if (!entrypoint->TryGetAddress()) {
        throw CompileError::NoEntrypoint();
    }

    labels->SetCodeSize(code_size);

    log << "[compiler]: substituting labels" << std::endl;

    FillLabels(files_data_map, labels, entrypoint);

    log << "[compiler]: successfully substituted labels" << std::endl;

    log << "[compiler]: preparing exec data" << std::endl;

    Exec::Data exec_data{
        .entrypoint    = *entrypoint->TryGetAddress(),
        .initial_stack = static_cast<arch::Word>(arch::kMemorySize - 1),
        .code          = std::vector<arch::Word>(),
        .constants     = std::vector<arch::Word>(),
    };

    for (const auto& data : files_data) {
        utils::vector::Append(exec_data.code, data.Code());
        utils::vector::Append(exec_data.constants, data.Constants());
    }

    log << "[compiler]: successfully prepared exec data" << std::endl;

    return exec_data;
}

////////////////////////////////////////////////////////////////////////////////
///                             Compile from file                            ///
////////////////////////////////////////////////////////////////////////////////

void Compiler::Impl::CompileImpl(const std::string& src,
                                 const std::string& dst,
                                 std::ostream& log) {
    log << "[compiler]: parsing includes" << std::endl;

    std::vector<std::unique_ptr<File>> files = IncludesManager().GetFiles(src);

    log << "[compiler]: successfully parsed includes, obtained " << files.size()
        << " files" << std::endl;

    Exec::Data data = PrepareExecData(files, log);

    std::string exec_path = dst;
    if (exec_path.empty()) {
        std::filesystem::path src_path(src);

        std::filesystem::path dst_path = src_path.parent_path();
        dst_path /= src_path.stem().replace_extension(exec::kDefaultExtension);

        exec_path = dst_path.string();
    }

    log << "[compiler]: writing exec data to file" << std::endl;

    Exec::Write(data, exec_path);

    log << "[compiler]: successfully written exec data to file" << std::endl;
}

void Compiler::Impl::MustCompile(const std::string& src,
                                 const std::string& dst,
                                 std::ostream& log) {
    using std::string_literals::operator""s;

    try {
        CompileImpl(src, dst, log);
    } catch (const errors::compiler::Error& e) {
        log << "[compiler]: error: " << e.what() << std::endl;
        throw e;
    } catch (const errors::Error& e) {
        log << "[compiler]: error: " << e.what() << std::endl;
        throw errors::compiler::Error(
            "error during compilation process "
            "(not directly related to the compilation itself): "s +
            e.what());
    } catch (const std::exception& e) {
        log << "[compiler]: unexpected exception: " << e.what() << std::endl;
        throw errors::compiler::Error("unexpected exception in compiler: "s +
                                      e.what());
    } catch (...) {
        log << "[compiler]: unexpected exception" << std::endl;
        throw errors::compiler::Error(
            "unexpected exception in compiler "
            "(no additional info can be provided)");
    }
}

void Compiler::Impl::Compile(const std::string& src,
                             const std::string& dst,
                             std::ostream& log) {
    try {
        MustCompile(src, dst, log);
    } catch (const errors::Error& e) {
        std::cerr << e.what() << std::endl;
    }
}

}  // namespace karma
