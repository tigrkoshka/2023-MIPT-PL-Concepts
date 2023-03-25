#include "compiler.hpp"

#include <algorithm>  // for max
#include <cstddef>    // for size_t
#include <string>     // for string
#include <thread>     // for hardware_concurrency

#include "compiler/impl.hpp"
#include "utils/logger.hpp"

namespace karma {

void Compiler::MustCompile(const std::string& src,
                           const std::string& dst,
                           size_t n_workers,
                           Logger log) {
    Impl::MustCompile(src, dst, n_workers, log.log);
}

void Compiler::Compile(const std::string& src,
                       const std::string& dst,
                       size_t n_workers,
                       Logger log) {
    Impl::Compile(src, dst, n_workers, log.log);
}

void Compiler::MustCompile(const std::string& src,
                           const std::string& dst,
                           Logger log) {
    MustCompile(src, dst, kDefaultWorkers, log);
}

void Compiler::Compile(const std::string& src,
                       const std::string& dst,
                       Logger log) {
    Compile(src, dst, kDefaultWorkers, log);
}

void Compiler::MustCompile(const std::string& src,
                           size_t n_workers,
                           Logger log) {
    MustCompile(src, "", n_workers, log);
}

void Compiler::Compile(const std::string& src, size_t n_workers, Logger log) {
    Compile(src, "", n_workers, log);
}

void Compiler::MustCompile(const std::string& src, Logger log) {
    MustCompile(src, "", kDefaultWorkers, log);
}

void Compiler::Compile(const std::string& src, Logger log) {
    Compile(src, "", kDefaultWorkers, log);
}

const size_t Compiler::kDefaultWorkers =
    std::max(1u, std::thread::hardware_concurrency());

}  // namespace karma
