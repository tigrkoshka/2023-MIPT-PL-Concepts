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

// NOLINTNEXTLINE(bugprone-exception-escape)
class Compiler::Data : detail::utils::traits::NonCopyableMovable {
   private:
    friend class FileCompiler;

   private:
    using InternalError = errors::compiler::InternalError::Builder;
    using CompileError  = errors::compiler::CompileError::Builder;

   private:
    void Merge(Data&& other);
    void CheckEntrypoint();
    void SubstituteLabels();

   public:
    static Data MergeAll(std::vector<Data>&);

    Exec::Data ToExecData(std::ostream& log) &&;

   private:
    Labels labels_;
    Entrypoint entrypoint_;
    std::vector<detail::specs::arch::Word> code_;
    std::vector<detail::specs::arch::Word> constants_;
};

}  // namespace karma
