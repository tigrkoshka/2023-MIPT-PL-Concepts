#include "file_data.hpp"

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

void Compiler::FileData::Merge(FileData&& other) {
    entrypoint.Merge(std::move(other.entrypoint));
    labels.Merge(std::move(other.labels), code.size(), constants.size());

    utils::vector::Append(code, other.code);
    utils::vector::Append(constants, other.constants);
}

void Compiler::FileData::CheckEntrypoint() {
    // even if the entrypoint was defines via a label,
    // the address should be recorded as 0 rather than std::nullopt
    if (!entrypoint.TryGetAddress()) {
        throw CompileError::NoEntrypoint();
    }
}

void Compiler::FileData::SubstituteLabels() {
    for (const auto& [label, usages] : labels.GetUsages()) {
        std::optional<size_t> definition_opt = labels.TryGetDefinition(label);
        if (!definition_opt) {
            throw CompileError::UndefinedLabel(
                {label, labels.GetUsageSample(label)});
        }

        auto definition = static_cast<arch::Address>(*definition_opt);
        for (size_t cmd_idx : usages) {
            // the address always occupies the last
            // bits of the command binary
            code[cmd_idx] |= definition;
        }
    }

    std::optional<std::string> entry = labels.TryGetEntrypointLabel();
    if (!entry) {
        return;
    }

    std::optional<size_t> definition = labels.TryGetDefinition(*entry);
    if (!definition) {
        std::optional<std::string> entry_pos = entrypoint.TryGetPos();
        if (!entry_pos) {
            // this is an internal error, because we have checked
            // the presence of address in the Entrypoint on an earlier
            // stage in the ToExecData method
            throw InternalError::NoEntrypointPosInLabelSubstitution();
        }

        throw CompileError::UndefinedLabel({*entry, *entry_pos});
    }

    entrypoint.SetAddress(static_cast<arch::Address>(*definition));
}

Compiler::FileData Compiler::FileData::MergeAll(std::vector<FileData>& all) {
    FileData res;
    for (auto& data : all) {
        res.Merge(std::move(data));
    }

    return res;
}

Exec::Data Compiler::FileData::ToExecData(std::ostream& log) && {
    CheckEntrypoint();
    labels.SetCodeSize(code.size());

    log << "[compiler]: substituting labels" << std::endl;
    SubstituteLabels();
    log << "[compiler]: successfully substituted labels" << std::endl;

    return {
        // we have checked the presence of the entrypoint in the beginning
        // of this function, and the labels substitution never unsets it
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        .entrypoint    = *entrypoint.TryGetAddress(),
        .initial_stack = static_cast<arch::Word>(arch::kMemorySize - 1),
        .code          = std::move(code),
        .constants     = std::move(constants),
    };
}

}  // namespace karma
