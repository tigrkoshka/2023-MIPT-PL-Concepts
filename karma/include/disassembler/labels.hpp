#pragma once

#include <optional>       // for optional
#include <string>         // for string
#include <unordered_map>  // for unordered_map

#include "disassembler/disassembler.hpp"
#include "exec/exec.hpp"
#include "specs/architecture.hpp"
#include "utils/traits.hpp"

namespace karma {

class Disassembler::Labels : detail::utils::traits::NonCopyableMovable {
   private:
    // definition -> label
    using LabelsMap =
        std::unordered_map<detail::specs::arch::Address, std::string>;

    using DisassembleError = errors::disassembler::DisassembleError::Builder;

   public:
    std::string RecordConstantLabel(detail::specs::arch::Address);
    void PrepareCommandLabels(const Exec::Data&);

    static std::string MainLabel();
    [[nodiscard]] std::optional<std::string> TryGetLabel(
        detail::specs::arch::Address) const;

   private:
    static const std::string kConstantLabelPrefix;
    static const std::string kCommandLabelPrefix;
    static const std::string kMainLabel;

    LabelsMap command_labels_;
    LabelsMap constant_labels_;
};

}  // namespace karma