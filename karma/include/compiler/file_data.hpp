#pragma once

#include <ostream>  // for ostream
#include <vector>   // for vector

#include "compiler/compiler.hpp"
#include "compiler/entrypoint.hpp"
#include "compiler/errors.hpp"
#include "compiler/labels.hpp"
#include "exec/exec.hpp"
#include "specs/architecture.hpp"
#include "utils/traits.hpp"

namespace karma {

class Compiler::FileData : detail::utils::traits::NonCopyableMovable {
   private:
    friend class FileCompiler;

   private:
    using InternalError = errors::compiler::InternalError::Builder;
    using CompileError  = errors::compiler::CompileError::Builder;

   private:
    void Merge(FileData&& other);
    void CheckEntrypoint();
    void SubstituteLabels();

   public:
    static FileData MergeAll(std::vector<FileData>&);

    Exec::Data ToExecData(std::ostream& log) &&;

   private:
    Labels labels;
    Entrypoint entrypoint;
    std::vector<detail::specs::arch::Word> code;
    std::vector<detail::specs::arch::Word> constants;
};

}  // namespace karma
