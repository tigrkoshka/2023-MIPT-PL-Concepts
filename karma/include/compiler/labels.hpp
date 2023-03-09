#pragma once

#include <cstddef>        // for size_t
#include <optional>       // for optional
#include <string>         // for string
#include <unordered_map>  // for unordered_map
#include <utility>        // for pair
#include <vector>         // for vector

namespace karma::compiler::detail {

class Labels {
   private:
    // definition, definition_line
    using Definition = std::pair<size_t, size_t>;

    // label -> definition
    using Definitions = std::unordered_map<std::string, Definition>;

    // line_number, position in code of the command which uses the label
    using Usages = std::vector<std::pair<size_t, size_t>>;

    // label -> LabelUsage
    using AllUsages = std::unordered_map<std::string, Usages>;

   public:
    static void CheckLabel(const std::string& label, size_t line);

    void SetCodeSize(size_t code_size);

    const std::string& GetLatest() const;
    size_t GetLatestLine() const;

    std::optional<size_t> TryGetDefinition(const std::string& label) const;
    std::optional<size_t> TryGetDefinitionLine(const std::string& label) const;

    void RecordLabelOccurrence(const std::string& label, size_t line);
    void RecordCommandLabel(size_t definition);
    void RecordConstantLabel(size_t definition);

    void RecordEntrypointLabel(const std::string& label);
    const std::optional<std::string>& TryGetEntrypointLabel();

    void RecordUsage(const std::string& label,
                     size_t line_number,
                     size_t command_number);
    const AllUsages& GetUsages();

   private:
    std::string latest_label_;
    size_t latest_label_line_{0};

    Definitions commands_labels_;
    Definitions constants_labels_;
    std::optional<std::string> entrypoint_label_;

    AllUsages usages_;

    size_t code_size_{};
};

}  // namespace karma::compiler::detail