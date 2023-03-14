#pragma once

#include <istream>        // for istream
#include <memory>         // for unique_ptr, shared_ptr
#include <string>         // for string
#include <unordered_map>  // for unordered_map
#include <vector>         // for vector

#include "entrypoint.hpp"
#include "exec/exec.hpp"
#include "exec_data.hpp"
#include "file.hpp"
#include "labels.hpp"
#include "utils/traits.hpp"

namespace karma::compiler::detail {

class Impl : karma::detail::utils::traits::NonCopyableMovable {
   private:
    using Files        = std::vector<std::unique_ptr<File>>;
    using FilesDataMap = std::unordered_map<const File*, ExecData*>;

   private:
    void FillLabels(const FilesDataMap& files_data);
    karma::detail::exec::Data PrepareExecData(const Files& files);

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

}  // namespace karma::compiler::detail
