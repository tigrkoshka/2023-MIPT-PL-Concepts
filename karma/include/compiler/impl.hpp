#pragma once

#include <cstdint>  // for uint32_t
#include <memory>   // for unique_ptr, shared_ptr
#include <ostream>  // for ostream
#include <span>     // for span
#include <string>   // for string
#include <vector>   // for vector

#include "compiler/compiler.hpp"
#include "compiler/errors.hpp"
#include "exec/exec.hpp"
#include "utils/traits.hpp"

namespace karma {

class Compiler::Impl : detail::utils::traits::Static {
   private:
    using Files = std::vector<std::unique_ptr<File>>;

   private:
    static void CompileRoutine(std::span<const std::unique_ptr<File>>,
                               std::span<Data>,
                               std::ostream& log);

    static Exec::Data PrepareExecData(const Files&,
                                      size_t n_workers,
                                      std::ostream& log);

    static void CompileImpl(const std::string& src,
                            const std::string& dst,
                            size_t n_workers,
                            std::ostream& log);

   public:
    static void MustCompile(const std::string& src,
                            const std::string& dst,
                            size_t n_workers,
                            std::ostream& log);
    static void Compile(const std::string& src,
                        const std::string& dst,
                        size_t n_workers,
                        std::ostream& log);
};

}  // namespace karma
