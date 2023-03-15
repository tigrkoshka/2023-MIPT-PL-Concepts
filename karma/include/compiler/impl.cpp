#include "impl.hpp"

#include <cstddef>     // for size_t
#include <exception>   // for exception
#include <filesystem>  // for path
#include <iostream>    // for cout
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

namespace utils = karma::detail::utils;

namespace arch = karma::detail::specs::arch;

namespace args = karma::detail::specs::cmd::args;

////////////////////////////////////////////////////////////////////////////////
///                            Labels substitution                           ///
////////////////////////////////////////////////////////////////////////////////

void Compiler::Impl::FillLabels(const FilesDataMap& files_data) {
    for (const auto& [label, usages] : labels_->GetUsages()) {
        std::optional<size_t> definition_opt = labels_->TryGetDefinition(label);
        if (!definition_opt) {
            throw CompileError::UndefinedLabel(
                {label, labels_->GetUsageSample(label)});
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

    if (std::optional<std::string> entry = labels_->TryGetEntrypointLabel()) {
        std::optional<size_t> definition = labels_->TryGetDefinition(*entry);
        if (!definition) {
            throw CompileError::UndefinedLabel(
                {*entry, *entrypoint_->TryGetPos()});
        }

        entrypoint_->SetAddress(static_cast<args::Address>(*definition));
    }
}

////////////////////////////////////////////////////////////////////////////////
///                                Prepare data                              ///
////////////////////////////////////////////////////////////////////////////////

Exec::Data Compiler::Impl::PrepareExecData(const Files& files) {
    // TODO: multithreading

    std::vector<ExecData> files_data(files.size());
    FilesDataMap files_data_map;
    size_t code_size = 0;
    for (size_t i = 0; i < files.size(); ++i) {
        FileCompiler file_compiler{files[i], labels_, entrypoint_};

        files_data[i]                  = std::move(file_compiler).PrepareData();
        files_data_map[files[i].get()] = &files_data[i];

        code_size += files_data[i].Code().size();
    }

    if (!entrypoint_->Seen()) {
        throw CompileError::NoEntrypoint();
    }

    labels_->SetCodeSize(code_size);

    FillLabels(files_data_map);

    Exec::Data exec_data{
        .entrypoint    = *entrypoint_->TryGetAddress(),
        .initial_stack = static_cast<arch::Word>(arch::kMemorySize - 1),
        .code          = std::vector<arch::Word>(),
        .constants     = std::vector<arch::Word>(),
    };

    for (const auto& data : files_data) {
        utils::vector::Append(exec_data.code, data.Code());
        utils::vector::Append(exec_data.constants, data.Constants());
    }

    return exec_data;
}

////////////////////////////////////////////////////////////////////////////////
///                             Compile from file                            ///
////////////////////////////////////////////////////////////////////////////////

void Compiler::Impl::MustCompile(const std::string& src,
                                 const std::string& dst) {
    std::vector<std::unique_ptr<File>> files = IncludesManager().GetFiles(src);

    Exec::Data data = PrepareExecData(files);

    std::string exec_path = dst;
    if (exec_path.empty()) {
        std::filesystem::path src_path(src);

        std::filesystem::path dst_path = src_path.parent_path();
        dst_path /= src_path.stem().replace_extension(
            karma::detail::specs::exec::kDefaultExtension);

        exec_path = dst_path.string();
    }

    Exec::Write(data, exec_path);
}

void Compiler::Impl::Compile(const std::string& src, const std::string& dst) {
    try {
        MustCompile(src, dst);
    } catch (const errors::compiler::Error& e) {
        std::cout << e.what() << std::endl;
    } catch (const errors::Error& e) {
        std::cout << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "compiler: unexpected exception: " << e.what()
                  << std::endl;
    } catch (...) {
        std::cout << "compiler: unexpected exception" << std::endl;
    }
}

}  // namespace karma
