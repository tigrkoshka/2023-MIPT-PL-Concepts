#pragma once

#include <memory>   // for unique_ptr, shared_ptr
#include <ostream>  // for ostream
#include <string>   // for string
#include <vector>   // for vector

#include "compiler/compiler.hpp"
#include "compiler/errors.hpp"
#include "exec/exec.hpp"
#include "utils/traits.hpp"

namespace karma {

class Compiler::Impl : detail::utils::traits::Static {
   private:
    static Exec::Data PrepareExecData(const std::vector<std::unique_ptr<File>>&,
                                      std::ostream& log);

    static void CompileImpl(const std::string& src,
                            const std::string& dst,
                            std::ostream& log);

   public:
    static void MustCompile(const std::string& src,
                            const std::string& dst,
                            std::ostream& log);
    static void Compile(const std::string& src,
                        const std::string& dst,
                        std::ostream& log);
};

}  // namespace karma
