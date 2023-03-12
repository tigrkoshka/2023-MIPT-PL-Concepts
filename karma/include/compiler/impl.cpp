#include "impl.hpp"

#include <cstddef>     // for size_t
#include <exception>   // for exception
#include <filesystem>  // for path
#include <iostream>    // for cout
#include <memory>      // for shared_ptr
#include <optional>    // for optional
#include <string>      // for string
#include <vector>      // for vector

#include "errors.hpp"
#include "exec/exec.hpp"
#include "exec_data.hpp"
#include "file_compiler.hpp"
#include "includes.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "specs/exec.hpp"

namespace karma::compiler::detail {

using errors::compiler::Error;
using errors::compiler::InternalError;
using errors::compiler::CompileError;

namespace arch = karma::detail::specs::arch;

namespace args = karma::detail::specs::cmd::args;

namespace exec = karma::detail::exec;

////////////////////////////////////////////////////////////////////////////////
///                            Labels substitution                           ///
////////////////////////////////////////////////////////////////////////////////

void Impl::FillLabels(const FilesDataMap& files_data) {
    for (const auto& [label, usages] : labels_.GetUsages()) {
        std::optional<size_t> definition_opt = labels_.TryGetDefinition(label);
        if (!definition_opt) {
            throw CompileError::UndefinedLabel(
                {label, labels_.GetUsageSample(label)});
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

    if (std::optional<std::string> entry = labels_.TryGetEntrypointLabel()) {
        std::optional<size_t> definition = labels_.TryGetDefinition(*entry);
        if (!definition) {
            throw CompileError::UndefinedLabel(
                {*entry, *entrypoint_.TryGetPos()});
        }

        entrypoint_.SetAddress(static_cast<args::Address>(*definition));
    }
}

////////////////////////////////////////////////////////////////////////////////
///                                Prepare data                              ///
////////////////////////////////////////////////////////////////////////////////

exec::Data Impl::PrepareExecData(const Files& files) {
    std::shared_ptr<Labels> labels_shared(&labels_);
    std::shared_ptr<Entrypoint> entrypoint_shared(&entrypoint_);

    // TODO: multithreading

    std::vector<ExecData> files_data(files.size());
    for (size_t i = 0; i < files.size(); ++i) {
        FileCompiler file_compiler{files[i], labels_shared, entrypoint_shared};
        files_data[i] = std::move(file_compiler).PrepareData();
    }

    if (!entrypoint_.Seen()) {
        throw CompileError::NoEntrypoint();
    }

    FilesDataMap files_data_map;
    for (size_t i = 0; i < files.size(); ++i) {
        files_data_map[files[i].get()] = &files_data[i];
    }

    size_t code_size = 0;
    for (const auto& data : files_data) {
        code_size += data.Code().size();
    }

    labels_.SetCodeSize(code_size);

    FillLabels(files_data_map);

    exec::Data exec_data{
        .entrypoint    = *entrypoint_.TryGetAddress(),
        .initial_stack = static_cast<arch::Word>(arch::kMemorySize - 1),
    };

    for (const auto& data : files_data) {
        exec_data.code.insert(exec_data.code.end(),
                              data.Code().begin(),
                              data.Code().end());
        exec_data.constants.insert(exec_data.constants.end(),
                                   data.Constants().begin(),
                                   data.Constants().end());
    }

    return exec_data;
}

////////////////////////////////////////////////////////////////////////////////
///                             Compile from file                            ///
////////////////////////////////////////////////////////////////////////////////

void Impl::MustCompile(const std::string& src, const std::string& dst) {
    std::vector<std::unique_ptr<File>> files = GetFiles(src);

    exec::Data data = PrepareExecData(files);

    std::string exec_path = dst;
    if (exec_path.empty()) {
        std::filesystem::path src_path(src);

        std::filesystem::path dst_path = src_path.parent_path();
        dst_path /= src_path.stem().replace_extension(
            karma::detail::specs::exec::kDefaultExtension);

        exec_path = dst_path.string();
    }

    exec::Write(data, exec_path);
}

void Impl::Compile(const std::string& src, const std::string& dst) {
    try {
        MustCompile(src, dst);
    } catch (const Error& e) {
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

}  // namespace karma::compiler::detail
