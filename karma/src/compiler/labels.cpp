#include "labels.hpp"

#include <cstddef>        // for size_t
#include <optional>       // for optional
#include <string>         // for string

#include "specs/syntax.hpp"

#include "errors.hpp"

namespace karma::compiler::detail {

using errors::compiler::CompileError;

namespace syntax = karma::detail::specs::syntax;

void Labels::CheckLabel(const std::string& label, size_t line) {
    if (label.empty()) {
        throw CompileError::EmptyLabel(line);
    }

    if (std::isdigit(label[0]) != 0) {
        throw CompileError::LabelStartsWithDigit(label, line);
    }

    for (char symbol : label) {
        if (!syntax::IsAllowedLabelChar(symbol)) {
            throw CompileError::InvalidLabelCharacter(symbol, label, line);
        }
    }
}

void Labels::SetCodeSize(size_t code_size) {
    code_size_ = code_size;
}

const std::string& Labels::GetLatest() const {
    return latest_label_;
}

size_t Labels::GetLatestLine() const {
    return latest_label_line_;
}

std::optional<size_t> Labels::TryGetDefinition(const std::string& label) const {
    if (commands_labels_.contains(label)) {
        return commands_labels_.at(label).first;
    }

    if (constants_labels_.contains(label)) {
        return code_size_ + constants_labels_.at(label).first;
    }

    return std::nullopt;
}

std::optional<size_t> Labels::TryGetDefinitionLine(
    const std::string& label) const {
    if (commands_labels_.contains(label)) {
        return commands_labels_.at(label).second;
    }

    if (constants_labels_.contains(label)) {
        return constants_labels_.at(label).second;
    }

    return std::nullopt;
}

void Labels::RecordLabelOccurrence(const std::string& label, size_t line) {
    latest_label_line_ = line;
    latest_label_      = label;
}

void Labels::RecordCommandLabel(size_t definition) {
    commands_labels_[latest_label_] = {definition, latest_label_line_};
}

void Labels::RecordConstantLabel(size_t definition) {
    constants_labels_[latest_label_] = {definition, latest_label_line_};
}

void Labels::RecordEntrypointLabel(const std::string& label) {
    entrypoint_label_ = label;
}

const std::optional<std::string>& Labels::TryGetEntrypointLabel() {
    return entrypoint_label_;
}

void Labels::RecordUsage(const std::string& label,
                         size_t line_number,
                         size_t command_number) {
    usages_[label].emplace_back(line_number, command_number);
}

const Labels::AllUsages& Labels::GetUsages() {
    return usages_;
}

}  // namespace karma::compiler::detail
