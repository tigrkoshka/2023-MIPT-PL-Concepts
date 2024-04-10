#include "impl.hpp"

#include <algorithm>   // for min
#include <exception>   // for exception
#include <filesystem>  // for path
#include <iostream>    // for ostream, cerr
#include <memory>      // for std::unique_ptr
#include <string>      // for string
#if defined(__GNUC__) && !defined(__clang__)
#include <syncstream>  // for osyncstream
#endif
#include <thread>  // for thread
#include <vector>  // for vector

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
    // TODO: delete this comment when Clang supports std::osyncstream,
    //       also delete #if here and in includes above
    //
    // C++20 std::osyncstream is currently only supported by GCC
#if defined(__GNUC__) && !defined(__clang__)
    std::osyncstream synced_log{log};
#else
    std::ostream& synced_log = log;
#endif

    for (size_t idx = 0; idx < src.size(); ++idx) {
        synced_log << "[compiler]: compiling " << src[idx]->Path() << '\n';

        dst[idx] = FileCompiler(src[idx]).PrepareData();

        synced_log << "[compiler]: successfully compiled " << src[idx]->Path()
                   << '\n';
    }
}

Exec::Data Compiler::Impl::PrepareExecData(const Files& files,
                                           size_t n_workers,
                                           std::ostream& log) {
    n_workers = std::min(n_workers, files.size());

    log << "[compiler]: compiling with " << n_workers << " workers\n";

    std::vector<Data> files_data(files.size());
    std::vector<std::thread> workers;

    const size_t rough_n_files_per_worker = files.size() / n_workers;
    const size_t n_workers_more_files     = files.size() % n_workers;

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
    log << "[compiler]: parsing includes\n";

    const Files files = IncludesManager().GetFiles(src);

    log << "[compiler]: successfully parsed includes, obtained " << files.size()
        << " files\n";

    const Exec::Data data = PrepareExecData(files, n_workers, log);

    std::string exec_path = dst;
    if (exec_path.empty()) {
        const std::filesystem::path src_path(src);

        std::filesystem::path dst_path = src_path.parent_path();
        dst_path /= src_path.stem().replace_extension(exec::kDefaultExtension);

        exec_path = dst_path.string();
    }

    log << "[compiler]: writing exec data to file\n";

    Exec::Write(data, exec_path);

    log << "[compiler]: successfully written exec data to file\n";
}

void Compiler::Impl::MustCompile(const std::string& src,
                                 const std::string& dst,
                                 size_t n_workers,
                                 std::ostream& log) {
    using std::string_literals::operator""s;

    try {
        CompileImpl(src, dst, n_workers, log);
    } catch (const errors::compiler::Error& e) {
        log << "[compiler]: error: " << e.what() << '\n';
        throw e;
    } catch (const errors::Error& e) {
        log << "[compiler]: error: " << e.what() << '\n';
        throw errors::compiler::Error(
            "error during compilation process "
            "(not directly related to the compilation itself): "s +
            e.what());
    } catch (const std::exception& e) {
        log << "[compiler]: unexpected exception: " << e.what() << '\n';
        throw errors::compiler::Error("unexpected exception in compiler: "s +
                                      e.what());
    } catch (...) {
        std::cout << "[compiler]: unexpected exception\n";
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
        std::cerr << e.what() << '\n';
    }
}

}  // namespace karma
