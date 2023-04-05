#include "labels.hpp"

#include <cctype>    // for is_digit
#include <cstddef>   // for size_t
#include <optional>  // for optional
#include <string>    // for string

#include "compiler/compiler.hpp"
#include "specs/syntax.hpp"

namespace karma {

namespace syntax = detail::specs::syntax;

void Compiler::Labels::CheckNotSeen(const std::string& label,
                                    const std::string& pos) {
    if (commands_labels_.contains(label)) {
        throw CompileError::LabelRedefinition(
            {label, pos},
            commands_labels_.at(label).second);
    }

    if (constants_labels_.contains(label)) {
        throw CompileError::LabelRedefinition(
            {label, pos},
            constants_labels_.at(label).second);
    }
}

void Compiler::Labels::Merge(Labels&& other,
                             size_t code_shift,
                             size_t constants_shift) {
    for (const auto& [label, definition] : other.commands_labels_) {
        RecordCommandLabel(label,
                           definition.first + code_shift,
                           definition.second);
    }

    for (const auto& [label, definition] : other.constants_labels_) {
        RecordConstantLabel(label,
                            definition.first + constants_shift,
                            definition.second);
    }

    if (other.entrypoint_label_) {
        // is both this->entrypoint_label_ and other.entrypoint_label_,
        // that can only mean that there were at least two entrypoints
        // in the program, the respective error will be thrown when merging
        // entrypoints, so there is no need to throw it here
        entrypoint_label_ = other.entrypoint_label_;
    }

    for (const auto& [label, usages] : other.usages_) {
        for (const size_t usage : usages) {
            usages_[label].push_back(usage + code_shift);
        }
    }

    for (const auto& [label, usage_sample] : other.usage_samples_) {
        if (!usage_samples_.contains(label)) {
            usage_samples_[label] = usage_sample;
        }
    }
}

void Compiler::Labels::CheckLabel(const std::string& label,
                                  const std::string& pos) {
    if (label.empty()) {
        throw CompileError::EmptyLabel(pos);
    }

    if (std::isdigit(label[0]) != 0) {
        throw CompileError::LabelStartsWithDigit({label, pos});
    }

    for (const char symbol : label) {
        if (!syntax::IsAllowedLabelChar(symbol)) {
            throw CompileError::InvalidLabelCharacter(symbol, {label, pos});
        }
    }
}

void Compiler::Labels::SetCodeSize(size_t code_size) {
    code_size_ = code_size;
}

std::optional<size_t> Compiler::Labels::TryGetDefinition(
    const std::string& label) const {
    if (commands_labels_.contains(label)) {
        return commands_labels_.at(label).first;
    }

    if (constants_labels_.contains(label)) {
        return code_size_ + constants_labels_.at(label).first;
    }

    return std::nullopt;
}

void Compiler::Labels::RecordCommandLabel(const std::string& label,
                                          size_t definition,
                                          const std::string& pos) {
    CheckNotSeen(label, pos);
    commands_labels_[label] = {definition, pos};
}

void Compiler::Labels::RecordConstantLabel(const std::string& label,
                                           size_t definition,
                                           const std::string& pos) {
    CheckNotSeen(label, pos);
    constants_labels_[label] = {definition, pos};
}

void Compiler::Labels::RecordEntrypointLabel(const std::string& label) {
    entrypoint_label_ = label;
}

std::optional<std::string> Compiler::Labels::TryGetEntrypointLabel() const {
    return entrypoint_label_;
}

void Compiler::Labels::RecordUsage(const std::string& label,
                                   size_t command_number,
                                   const std::string& pos) {
    usages_[label].push_back(command_number);

    if (!usage_samples_.contains(label)) {
        usage_samples_[label] = pos;
    }
}

const Compiler::Labels::Usages& Compiler::Labels::GetUsages() const {
    return usages_;
}

std::string Compiler::Labels::GetUsageSample(const std::string& label) const {
    return usage_samples_.at(label);
}

}  // namespace karma
