#include "data.hpp"

#include <cstddef>   // for size_t
#include <optional>  // for optional
#include <ostream>   // for ostream
#include <utility>   // for move
#include <vector>    // for vector

#include "exec/exec.hpp"
#include "specs/architecture.hpp"
#include "utils/vector.hpp"

namespace karma {

namespace arch  = detail::specs::arch;
namespace utils = detail::utils;

void Compiler::Data::Merge(Data&& other) {
    // move the whole parameter to avoid invalid state
    // (this is mandated by cppcoreguidelines)
    Data used{std::move(other)};

    entrypoint_.Merge(std::move(used.entrypoint_));
    labels_.Merge(std::move(used.labels_), code_.size(), constants_.size());

    utils::vector::Append(code_, used.code_);
    utils::vector::Append(constants_, used.constants_);
}

void Compiler::Data::CheckEntrypoint() {
    // even if the entrypoint was defines via a label,
    // the address should be recorded as 0 rather than std::nullopt
    if (!entrypoint_.TryGetAddress()) {
        throw CompileError::NoEntrypoint();
    }
}

void Compiler::Data::SubstituteLabels() {
    for (const auto& [label, usages] : labels_.GetUsages()) {
        std::optional<size_t> definition_opt = labels_.TryGetDefinition(label);
        if (!definition_opt) {
            throw CompileError::UndefinedLabel(
                {label, labels_.GetUsageSample(label)});
        }

        auto definition = static_cast<arch::Address>(*definition_opt);
        for (const size_t cmd_idx : usages) {
            // the address always occupies the last
            // bits of the command binary
            code_[cmd_idx] |= definition;
        }
    }

    std::optional<std::string> entry = labels_.TryGetEntrypointLabel();
    if (!entry) {
        return;
    }

    std::optional<size_t> definition = labels_.TryGetDefinition(*entry);
    if (!definition) {
        std::optional<std::string> entry_pos = entrypoint_.TryGetPos();
        if (!entry_pos) {
            // this is an internal error, because we have checked
            // the presence of address in the Entrypoint on an earlier
            // stage in the ToExecData method
            throw InternalError::NoEntrypointPosInLabelSubstitution();
        }

        throw CompileError::UndefinedLabel({*entry, *entry_pos});
    }

    entrypoint_.SetAddress(static_cast<arch::Address>(*definition));
}

Compiler::Data Compiler::Data::MergeAll(std::vector<Data>& all) {
    Data res;
    for (auto& data : all) {
        res.Merge(std::move(data));
    }

    return res;
}

Exec::Data Compiler::Data::ToExecData(std::ostream& log) && {
    CheckEntrypoint();
    labels_.SetCodeSize(code_.size());

    log << "[compiler]: substituting labels\n";
    SubstituteLabels();
    log << "[compiler]: successfully substituted labels\n";

    return {
        // we have checked the presence of the entrypoint in the beginning
        // of this function, and the labels substitution never unsets it
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        *entrypoint_.TryGetAddress(),
        static_cast<arch::Word>(arch::kMemorySize - 1),
        std::move(code_),
        std::move(constants_),
    };
}

}  // namespace karma
