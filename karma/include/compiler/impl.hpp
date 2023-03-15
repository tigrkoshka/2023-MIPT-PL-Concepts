#pragma once

#include <istream>        // for istream
#include <memory>         // for unique_ptr, shared_ptr
#include <string>         // for string
#include <unordered_map>  // for unordered_map
#include <vector>         // for vector

#include "compiler/compiler.hpp"
#include "compiler/entrypoint.hpp"
#include "compiler/exec_data.hpp"
#include "compiler/file.hpp"
#include "compiler/labels.hpp"
#include "exec/exec.hpp"
#include "utils/traits.hpp"

namespace karma {

class Compiler::Impl : karma::detail::utils::traits::NonCopyableMovable {
   private:
    using CompileError = errors::compiler::CompileError::Builder;

    using Files        = std::vector<std::unique_ptr<File>>;
    using FilesDataMap = std::unordered_map<const File*, ExecData*>;

   private:
    void FillLabels(const FilesDataMap& files_data);
    karma::Exec::Data PrepareExecData(const Files& files);

   public:
    Impl()
        : labels_(std::make_shared<Labels>()),
          entrypoint_(std::make_shared<Entrypoint>()){};

    void MustCompile(const std::string& src, const std::string& dst);
    void Compile(const std::string& src, const std::string& dst);

   private:
    std::shared_ptr<Labels> labels_;
    std::shared_ptr<Entrypoint> entrypoint_;
};

}  // namespace karma
