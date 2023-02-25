#include "compiler.hpp"

namespace karma {

using namespace detail::specs;  // NOLINT(google-build-using-namespace)

void Compiler::CheckLabel(const std::string& label, size_t line) {
    if (label.empty()) {
        throw CompileError::EmptyLabel(line);
    }

    if (std::isdigit(label[0]) != 0) {
        throw CompileError::LabelStartsWithDigit(label, line);
    }

    for (char symbol : label) {
        if (std::isalnum(symbol) == 0) {
            throw CompileError::InvalidLabelCharacter(symbol, label, line);
        }
    }
}

std::optional<Address> Compiler::ParseAddress(const std::string& word,
                                              size_t line) {
    try {
        size_t pos{};
        int32_t operand = std::stoi(word, &pos, 0);

        if (pos != word.size()) {
            // this means that there is a number in the beginning of word,
            // but the whole word is not a number
            //
            // assume the programmer meant it as a label,
            // but a label cannot start with a digit
            //
            // NOTE: we do allow '0', '0x' and '0X' prefixed numbers,
            //       but in all those cases word still starts with a digit ('0')
            //       and thus is not a valid label
            throw CompileError::LabelStartsWithDigit(word, line);
        }

        if (operand < 0) {
            throw CompileError::AddressNegative(word, line);
        }

        auto address = static_cast<Address>(operand);
        if (address > kMemorySize) {
            throw CompileError::AddressOutOfMemory(word, line);
        }

        return address;
    } catch (const std::invalid_argument&) {
        // this implies that the word does not start with a digit,
        // so we assume it's a label
        CheckLabel(word, line);
        return std::nullopt;
    } catch (const std::out_of_range&) {
        throw CompileError::AddressOutOfMemory(word, line);
    }
}

int32_t Compiler::ParseImmediateOperand(uint32_t bit_size,
                                        const std::string& word,
                                        size_t n_line) {
    const int32_t min = -static_cast<int32_t>(1u << (bit_size - 1));
    const int32_t max = static_cast<int32_t>(1u << (bit_size - 1)) - 1;

    try {
        size_t pos{};
        int32_t operand = std::stoi(word, &pos, 0);

        if (pos != word.size()) {
            throw CompileError::ImmediateNotANumber(word, n_line);
        }

        if (operand < min) {
            throw CompileError::ImmediateLessThanMin(min, word, n_line);
        }

        if (operand > max) {
            throw CompileError::ImmediateMoreThanMax(max, word, n_line);
        }

        return operand;
    } catch (const std::invalid_argument&) {
        throw CompileError::ImmediateNotANumber(word, n_line);
    } catch (const std::out_of_range&) {
        throw CompileError::ImmediateOutOfRange(word, n_line);
    }
}

void Compiler::Compile(std::istream& code, std::ostream& bin) {}

}  // namespace karma
