#include "labels.hpp"

#include <cctype>    // for is_digit
#include <cstddef>   // for size_t
#include <optional>  // for optional
#include <string>    // for string

#include "compiler/compiler.hpp"
#include "compiler/file.hpp"
#include "specs/syntax.hpp"

namespace karma {

namespace syntax = karma::detail::specs::syntax;

void Compiler::Compiler::Labels::CheckLabel(const std::string& label,
                                            const std::string& pos) {
    if (label.empty()) {
        throw CompileError::EmptyLabel(pos);
    }

    if (std::isdigit(label[0]) != 0) {
        throw CompileError::LabelStartsWithDigit({label, pos});
    }

    for (char symbol : label) {
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

std::optional<std::string> Compiler::Labels::TryGetPos(
    const std::string& label) const {
    if (commands_labels_.contains(label)) {
        return commands_labels_.at(label).second;
    }

    if (constants_labels_.contains(label)) {
        return constants_labels_.at(label).second;
    }

    return std::nullopt;
}

void Compiler::Labels::RecordCommandLabel(const std::string& label,
                                          size_t definition,
                                          const std::string& pos) {
    commands_labels_[label] = {definition, pos};
}

void Compiler::Labels::RecordConstantLabel(const std::string& label,
                                           size_t definition,
                                           const std::string& pos) {
    constants_labels_[label] = {definition, pos};
}

void Compiler::Labels::RecordEntrypointLabel(const std::string& label) {
    entrypoint_label_ = label;
}

std::optional<std::string> Compiler::Labels::TryGetEntrypointLabel() const {
    return entrypoint_label_;
}

void Compiler::Labels::RecordUsage(const std::string& label,
                                   const File* file,
                                   size_t command_number) {
    usages_[label][file].push_back(command_number);

    if (!usage_samples_.contains(label)) {
        usage_samples_[label] = file->Where();
    }
}

const Compiler::Labels::AllUsages& Compiler::Labels::GetUsages() const {
    return usages_;
}

std::string Compiler::Labels::GetUsageSample(const std::string& label) const {
    return usage_samples_.at(label);
}

}  // namespace karma
