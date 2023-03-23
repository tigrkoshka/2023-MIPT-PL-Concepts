#include "impl.hpp"

#include <exception>   // for exception
#include <filesystem>  // for path
#include <iostream>    // for ostream, cerr
#include <memory>      // for std::unique_ptr
#include <string>      // for string
#include <vector>      // for vector

#include "compiler/compiler.hpp"
#include "compiler/file.hpp"
#include "compiler/file_compiler.hpp"
#include "compiler/file_data.hpp"
#include "compiler/includes.hpp"
#include "exec/exec.hpp"
#include "specs/exec.hpp"
#include "utils/error.hpp"

namespace karma {

namespace exec = detail::specs::exec;

////////////////////////////////////////////////////////////////////////////////
///                                Prepare data                              ///
////////////////////////////////////////////////////////////////////////////////

Exec::Data Compiler::Impl::PrepareExecData(
    const std::vector<std::unique_ptr<File>>& files, std::ostream& log) {
    // TODO: multithreading
    std::vector<FileData> files_data;
    for (const auto& file : files) {
        log << "[compiler]: compiling " << file->Path() << std::endl;

        files_data.push_back(FileCompiler(file).PrepareData());

        log << "[compiler]: successfully compiled " << file->Path()
            << std::endl;
    }

    return FileData::MergeAll(files_data).ToExecData(log);
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
