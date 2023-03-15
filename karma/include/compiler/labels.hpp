#pragma once

#include <cstddef>        // for size_t
#include <optional>       // for optional
#include <string>         // for string
#include <unordered_map>  // for unordered_map
#include <utility>        // for pair
#include <vector>         // for vector

#include "compiler/compiler.hpp"
#include "compiler/errors.hpp"
#include "compiler/file.hpp"
#include "utils/traits.hpp"

namespace karma {

class Compiler::Labels : detail::utils::traits::NonCopyableMovable {
   private:
    using CompileError = errors::compiler::CompileError::Builder;

    // definition, where
    using Definition  = std::pair<size_t, std::string>;
    using Definitions = std::unordered_map<std::string, Definition>;

    // number of command in file
    using FileUsages = std::vector<size_t>;
    using Usages     = std::unordered_map<const File*, FileUsages>;
    using AllUsages  = std::unordered_map<std::string, Usages>;

    // label -> usage sample (line description from File.Where)
    using UsageSamples = std::unordered_map<std::string, std::string>;

   public:
    static void CheckLabel(const std::string& label, const std::string& pos);

    void SetCodeSize(size_t code_size);

    [[nodiscard]] std::optional<size_t> TryGetDefinition(
        const std::string& label) const;
    [[nodiscard]] std::optional<std::string> TryGetPos(
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
                     const File* file,
                     size_t command_number);
    [[nodiscard]] const AllUsages& GetUsages() const;
    [[nodiscard]] std::string GetUsageSample(const std::string& label) const;

   private:
    Definitions commands_labels_;
    Definitions constants_labels_;
    std::optional<std::string> entrypoint_label_;

    AllUsages usages_;
    UsageSamples usage_samples_;

    size_t code_size_{};
};

}  // namespace karma