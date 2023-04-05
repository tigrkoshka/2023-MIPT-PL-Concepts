#pragma once

#include <cstddef>        // for size_t
#include <optional>       // for optional
#include <string>         // for string
#include <unordered_map>  // for unordered_map
#include <utility>        // for pair
#include <vector>         // for vector

#include "compiler/compiler.hpp"
#include "compiler/errors.hpp"
#include "utils/traits.hpp"

namespace karma {

// NOLINTNEXTLINE(bugprone-exception-escape)
class Compiler::Labels : detail::utils::traits::NonCopyableMovable {
   private:
    using CompileError = errors::compiler::CompileError::Builder;

    // definition, where
    using Definition  = std::pair<size_t, std::string>;
    using Definitions = std::unordered_map<std::string, Definition>;

    // indices of commands in file where a label is used
    using Usages = std::unordered_map<std::string, std::vector<size_t>>;

    // label -> usage sample (line description from File.Where)
    using UsageSamples = std::unordered_map<std::string, std::string>;

   private:
    void CheckNotSeen(const std::string& label, const std::string& pos);

   public:
    void Merge(Labels&& other, size_t code_shift, size_t constants_shift);

    static void CheckLabel(const std::string& label, const std::string& pos);

    void SetCodeSize(size_t code_size);

    [[nodiscard]] std::optional<size_t> TryGetDefinition(
        const std::string& label) const;

    void RecordCommandLabel(const std::string& label,
                            size_t definition,
                            const std::string& pos);
    void RecordConstantLabel(const std::string& label,
                             size_t definition,
                             const std::string& pos);

    void RecordEntrypointLabel(const std::string& label);
    [[nodiscard]] std::optional<std::string> TryGetEntrypointLabel() const;

    void RecordUsage(const std::string& label,
                     size_t command_number,
                     const std::string& pos);
    [[nodiscard]] const Usages& GetUsages() const;
    [[nodiscard]] std::string GetUsageSample(const std::string& label) const;

   private:
    Definitions commands_labels_;
    Definitions constants_labels_;
    std::optional<std::string> entrypoint_label_;

    Usages usages_;
    UsageSamples usage_samples_;

    size_t code_size_{};
};

}  // namespace karma
