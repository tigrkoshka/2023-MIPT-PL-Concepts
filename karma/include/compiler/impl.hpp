#pragma once

#include <istream>        // for istream
#include <memory>         // for unique_ptr, shared_ptr
#include <string>         // for string
#include <unordered_map>  // for unordered_map
#include <vector>         // for vector

#include "compiler/compiler.hpp"
#include "compiler/entrypoint.hpp"
#include "compiler/errors.hpp"
#include "compiler/exec_data.hpp"
#include "compiler/file.hpp"
#include "compiler/labels.hpp"
#include "exec/exec.hpp"
#include "utils/traits.hpp"

namespace karma {

class Compiler::Impl : karma::detail::utils::traits::Static {
   private:
    using CompileError = errors::compiler::CompileError::Builder;

    using Files        = std::vector<std::unique_ptr<File>>;
    using FilesDataMap = std::unordered_map<const File*, ExecData*>;

   private:
    static void FillLabels(const FilesDataMap&,
                           std::shared_ptr<Labels>&,
                           std::shared_ptr<Entrypoint>&);
    static karma::Exec::Data PrepareExecData(const Files&);

   public:
    static void MustCompile(const std::string& src, const std::string& dst);
    static void Compile(const std::string& src, const std::string& dst);
};

}  // namespace karma
