#include "compiler.hpp"

#include <algorithm>  // for max
#include <cstddef>    // for size_t
#include <ostream>    // for ostream
#include <string>     // for string
#include <thread>     // for hardware_concurrency

#include "compiler/impl.hpp"
#include "utils/logger.hpp"

namespace karma {

void Compiler::MustCompile(const std::string& src,
                           const std::string& dst,
                           size_t n_workers,
                           std::ostream& log) {
    Impl::MustCompile(src, dst, n_workers, log);
}

void Compiler::Compile(const std::string& src,
                       const std::string& dst,
                       size_t n_workers,
                       std::ostream& log) {
    Impl::Compile(src, dst, n_workers, log);
}

void Compiler::MustCompile(const std::string& src,
                           const std::string& dst,
                           std::ostream& log) {
    MustCompile(src, dst, kDefaultWorkers, log);
}

void Compiler::Compile(const std::string& src,
                       const std::string& dst,
                       std::ostream& log) {
    Compile(src, dst, kDefaultWorkers, log);
}

void Compiler::MustCompile(const std::string& src,
                           size_t n_workers,
                           std::ostream& log) {
    MustCompile(src, "", n_workers, log);
}

void Compiler::Compile(const std::string& src,
                       size_t n_workers,
                       std::ostream& log) {
    Compile(src, "", n_workers, log);
}

void Compiler::MustCompile(const std::string& src, std::ostream& log) {
    MustCompile(src, "", kDefaultWorkers, log);
}

void Compiler::Compile(const std::string& src, std::ostream& log) {
    Compile(src, "", kDefaultWorkers, log);
}

std::ostream& Compiler::no_op_logger = detail::utils::logger::no_op;
const size_t Compiler::kDefaultWorkers =
    std::max(1u, std::thread::hardware_concurrency());

}  // namespace karma
