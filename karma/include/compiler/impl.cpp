#include "impl.hpp"

#include <exception>   // for exception
#include <filesystem>  // for path
#include <iostream>    // for ostream, cerr
#include <memory>      // for std::unique_ptr
#include <string>      // for string
#include <syncstream>  // for osyncstream
#include <thread>      // for thread
#include <vector>      // for vector

#include "compiler/compiler.hpp"
#include "compiler/data.hpp"
#include "compiler/file.hpp"
#include "compiler/file_compiler.hpp"
#include "compiler/includes.hpp"
#include "exec/exec.hpp"
#include "specs/exec.hpp"
#include "utils/error.hpp"

namespace karma {

namespace exec = detail::specs::exec;

////////////////////////////////////////////////////////////////////////////////
///                                Prepare data                              ///
////////////////////////////////////////////////////////////////////////////////

void Compiler::Impl::CompileRoutine(std::span<const std::unique_ptr<File>> src,
                                    std::span<Data> dst,
                                    std::ostream& log) {
    // TODO: delete this comment when Clang supports std::osyncstream
    //
    // C++20 std::osyncstream is currently only supported by GCC, and not Clang
    // to compile with Clang delete this line (though the logs may interfere
    // with each other) and delete the #include <syncstream> above
    std::osyncstream synced_log{log};

    for (size_t idx = 0; idx < src.size(); ++idx) {
        synced_log << "[compiler]: compiling " << src[idx]->Path() << std::endl;

        dst[idx] = FileCompiler(src[idx]).PrepareData();

        synced_log << "[compiler]: successfully compiled " << src[idx]->Path()
                   << std::endl;
    }
}

Exec::Data Compiler::Impl::PrepareExecData(const Files& files,
                                           size_t n_workers,
                                           std::ostream& log) {
    if (files.size() < n_workers) {
        n_workers = files.size();
    }

    log << "[compiler]: compiling with " << n_workers << " workers"
        << std::endl;

    std::vector<Data> files_data(files.size());
    std::vector<std::thread> workers;

    size_t rough_n_files_per_worker = files.size() / n_workers;
    size_t n_workers_more_files     = files.size() % n_workers;

    using DiffT      = Files::iterator::difference_type;
    auto files_start = files.begin();
    auto data_start  = files_data.begin();

    for (size_t idx = 0; idx < n_workers; ++idx) {
        size_t n_files_per_worker = rough_n_files_per_worker;
        if (idx < n_workers_more_files) {
            ++n_files_per_worker;
        }

        workers.emplace_back(
            [files_start, data_start, n_files_per_worker, &log]() {
                CompileRoutine({files_start, n_files_per_worker},
                               {data_start, n_files_per_worker},
                               log);
            });

        files_start += static_cast<DiffT>(n_files_per_worker);
        data_start += static_cast<DiffT>(n_files_per_worker);
    }

    for (auto& worker : workers) {
        worker.join();
    }

    return Data::MergeAll(files_data).ToExecData(log);
}

////////////////////////////////////////////////////////////////////////////////
///                             Compile from file                            ///
////////////////////////////////////////////////////////////////////////////////

void Compiler::Impl::CompileImpl(const std::string& src,
                                 const std::string& dst,
                                 size_t n_workers,
                                 std::ostream& log) {
    log << "[compiler]: parsing includes" << std::endl;

    Files files = IncludesManager().GetFiles(src);

    log << "[compiler]: successfully parsed includes, obtained " << files.size()
        << " files" << std::endl;

    Exec::Data data = PrepareExecData(files, n_workers, log);

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
                                 size_t n_workers,
                                 std::ostream& log) {
    using std::string_literals::operator""s;

    try {
        CompileImpl(src, dst, n_workers, log);
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
                             size_t n_workers,
                             std::ostream& log) {
    try {
        MustCompile(src, dst, n_workers, log);
    } catch (const errors::Error& e) {
        std::cerr << e.what() << std::endl;
    }
}

}  // namespace karma
