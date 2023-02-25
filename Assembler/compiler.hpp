#pragma once

#include <algorithm>
#include <iomanip>
#include <istream>
#include <iterator>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "specs.hpp"
#include "utils.hpp"

namespace karma {

namespace util = detail::util;

using namespace detail::specs;

class Compiler {
   private:
    using CommandCode = detail::specs::CommandCode;
    using Register    = detail::specs::Register;
    using Address     = detail::specs::Address;

   private:
    struct InternalError : std::runtime_error {
       private:
        explicit InternalError(const std::string& message, size_t line)
            : std::runtime_error("internal compiler error at line " +
                                 std::to_string(line) + ":" + message) {}

       public:
        static InternalError UnknownCommandCode(CommandCode command_code,
                                                size_t line) {
            std::ostringstream ss;
            ss << "unknown command code " << command_code;
            return InternalError{ss.str(), line};
        }

        static InternalError UnknownCommandFormat(CommandFormat format,
                                                  size_t line) {
            std::ostringstream ss;
            ss << "unknown command format " << format;
            return InternalError{ss.str(), line};
        }
    };

    struct CompileError : std::runtime_error {
       private:
        explicit CompileError(const std::string& message, size_t line)
            : std::runtime_error("compile error at line " +
                                 std::to_string(line) + ":" + message) {}

       public:
        // command

        static CompileError UnknownCommand(const std::string& command,
                                           size_t line) {
            std::ostringstream ss;
            ss << "unknown command " << std::quoted(command);
            return CompileError{ss.str(), line};
        }

        // labels

        static CompileError EmptyLabel(size_t line) {
            return CompileError{"empty label name", line};
        }

        static CompileError LabelStartsWithDigit(const std::string& label,
                                                 size_t line) {
            std::ostringstream ss;
            ss << "label " << std::quoted(label) << " starts with a digit";
            return CompileError{ss.str(), line};
        }

        static CompileError InvalidLabelCharacter(char invalid,
                                                  const std::string& label,
                                                  size_t line) {
            std::ostringstream ss;
            ss << "label " << std::quoted(label)
               << " contains an invalid character: \'" << invalid
               << "\' (only latin letters and digits are allowed)";
            return CompileError{ss.str(), line};
        }

        static CompileError UndefinedLabel(const std::string& label,
                                           size_t line) {
            std::ostringstream ss;
            ss << "label " << std::quoted(label) << " is not defined";
            return CompileError{ss.str(), line};
        }

        // register

        static CompileError UnknownRegister(const std::string& reg,
                                            size_t line) {
            std::ostringstream ss;
            ss << "unknown register " << std::quoted(reg);
            return CompileError{ss.str(), line};
        }

        // address

        static CompileError AddressNegative(const std::string& address,
                                            size_t line) {
            std::ostringstream ss;
            ss << "the address operand " << std::quoted(address, ')')
               << " must not be negative";
            return CompileError{ss.str(), line};
        }

        static CompileError AddressOutOfMemory(const std::string& address,
                                               size_t line) {
            std::ostringstream ss;
            ss << "the address operand " << std::quoted(address, ')')
               << " exceeds the memory size";
            return CompileError{ss.str(), line};
        }

        // immediate

        static CompileError ImmediateNotANumber(const std::string& immediate,
                                                size_t line) {
            std::ostringstream ss;
            ss << "the immediate operand is not a number: "
               << std::quoted(immediate);
            return CompileError{ss.str(), line};
        }

        static CompileError ImmediateLessThanMin(int32_t min,
                                                 const std::string& immediate,
                                                 size_t line) {
            std::ostringstream ss;
            ss << "the immediate operand is less than the allowed minimum ("
               << min << "): " << immediate;
            return CompileError{ss.str(), line};
        }

        static CompileError ImmediateMoreThanMax(int32_t max,
                                                 const std::string& immediate,
                                                 size_t line) {
            std::ostringstream ss;
            ss << "the immediate operand is less than the allowed maximum ("
               << max << "): " << immediate;
            return CompileError{ss.str(), line};
        }

        static CompileError ImmediateOutOfRange(const std::string& immediate,
                                                size_t line) {
            std::ostringstream ss;
            ss << "the immediate operand is out of range " << immediate;
            return CompileError{ss.str(), line};
        }

        // RM

        static CompileError RMCommandNoRegister(size_t line) {
            return CompileError{
                "register not specified for RM format command",
                line,
            };
        }

        static CompileError RMCommandNoAddress(size_t line) {
            return CompileError{
                "memory address not specified for RM format command",
                line,
            };
        }

        // RR

        static CompileError RRCommandNoReceiver(size_t line) {
            return CompileError{
                "receiver register not specified for RR format command",
                line,
            };
        }

        static CompileError RRCommandNoSource(size_t line) {
            return CompileError{
                "source register not specified for RR format command",
                line,
            };
        }

        static CompileError RRCommandNoModifier(size_t line) {
            return CompileError{
                "source modifier operand not specified for RR format command, "
                "specify 0 for no modification",
                line,
            };
        }

        // RI

        static CompileError RICommandNoRegister(size_t line) {
            return CompileError{
                "register not specified for RI format command",
                line,
            };
        }

        static CompileError RICommandNoImmediate(size_t line) {
            return CompileError{
                "immediate operand not specified for RI format command",
                line,
            };
        }

        // J

        static CompileError JCommandNoAddress(size_t line) {
            return CompileError{
                "memory address not specified for J format command",
                line,
            };
        }

        // More

        static CompileError ExtraWords(CommandFormat format,
                                       const std::string& extra,
                                       size_t line) {
            std::ostringstream ss;
            ss << "the line starts with a valid command (format "
               << kFormatToString.at(format)
               << "), but has unexpected words at the end (starting from "
               << std::quoted(extra) << ")";
            return CompileError{ss.str(), line};
        }
    };

   private:
    // TODO: doc comment (include about throws)
    static void CheckLabel(const std::string& label, size_t line) {
        if (label.empty()) {
            throw CompileError::EmptyLabel(line);
        }

        if (std::isdigit(label[0]) != 0) {
            throw CompileError::LabelStartsWithDigit(label, line);
        }

        for (char symbol : label) {
            if (std::islower(symbol) == 0 && std::isdigit(symbol) == 0) {
                throw CompileError::InvalidLabelCharacter(symbol, label, line);
            }
        }
    }

    // TODO: doc comment
    // throws on any error
    //
    // returns numeric address value if parsed,
    // else should be considered a label
    static std::optional<Address> ParseAddress(const std::string& word,
                                               size_t n_line) {
        try {
            size_t pos{};
            int32_t operand = std::stoi(word, &pos, 0);

            if (pos != word.size()) {
                // assume the programmer meant it as a label,
                // but a label cannot start with a digit
                throw CompileError::LabelStartsWithDigit(word, n_line);
            }

            if (operand < 0) {
                throw CompileError::AddressNegative(word, n_line);
            }

            // the address value could've fit to 32 bits,
            // but still be bigger than memory size,
            // in that case std::stoi does not throw
            // const std::out_of_range, but we should
            // still throw a compilation error
            if (static_cast<size_t>(operand) > kMemorySize) {
                throw CompileError::AddressOutOfMemory(word, n_line);
            }

            return static_cast<Address>(operand);
        } catch (const std::invalid_argument&) {
            // assume it's a label

            // throws a compile error if the label is invalid
            CheckLabel(word, n_line);
            return std::nullopt;
        } catch (const std::out_of_range&) {
            throw CompileError::AddressOutOfMemory(word, n_line);
        }
    }

    // TODO: doc comment
    // throws on any error
    //
    // returns numeric value if parsed
    static int32_t ParseImmediateOperand(uint32_t bit_size,
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

    static void CompileImpl(std::istream& code) {
        // TODO: major decomposition

        // TODO: think on checking that a label cannot occur after all code

        // label -> command_number
        std::unordered_map<std::string, Address> label_definitions;

        // label -> line_number, command_number
        std::unordered_map<std::string, std::vector<std::pair<size_t, size_t>>>
            label_usages;

        std::vector<uint32_t> compiled;

        size_t n_line     = 0;
        Address n_command = 0;
        for (std::string line; std::getline(code, line); ++n_line) {
            // ignore comments
            line.resize(std::min(line.find(kCommentSep), line.size()));

            std::istringstream ss(std::move(line));
            std::string word;

            if (!(ss >> word)) {
                continue;
            }

            if (word.back() == kLabelEnd) {
                word.resize(word.size() - 1);

                // throws a compile error if the label is invalid
                CheckLabel(word, n_line);

                label_definitions[word] = n_command;

                if (!(ss >> word)) {
                    continue;
                }
            }

            if (!kCommandToCode.contains(word)) {
                throw CompileError::UnknownCommand(word, n_line);
            }

            CommandCode command_code = kCommandToCode.at(word);
            if (!kCodeToFormat.contains(command_code)) {
                throw InternalError::UnknownCommandCode(command_code, n_line);
            }

            // TODO: decompose getting a register

            CommandFormat format = kCodeToFormat.at(command_code);
            switch (format) {
                case RM: {
                    if (!(ss >> word)) {
                        throw CompileError::RMCommandNoRegister(n_line);
                    }

                    if (!kRegisterToNum.contains(word)) {
                        throw CompileError::UnknownRegister(word, n_line);
                    }

                    Register reg = kRegisterToNum.at(word);

                    if (!(ss >> word)) {
                        throw CompileError::RMCommandNoAddress(n_line);
                    }

                    std::optional<Address> address = ParseAddress(word, n_line);
                    if (address == std::nullopt) {
                        label_usages[word].emplace_back(n_line, n_command);

                        // will be set later during labels substitution
                        address = 0;
                    }

                    compiled.push_back((command_code << cmd::kCodeShift) |
                                       (reg << cmd::kRecvShift) | *address);

                    break;
                }

                case RR: {
                    if (!(ss >> word)) {
                        throw CompileError::RRCommandNoReceiver(n_line);
                    }

                    if (!kRegisterToNum.contains(word)) {
                        throw CompileError::UnknownRegister(word, n_line);
                    }

                    Register recv = kRegisterToNum.at(word);

                    if (!(ss >> word)) {
                        throw CompileError::RRCommandNoSource(n_line);
                    }

                    if (!kRegisterToNum.contains(word)) {
                        throw CompileError::UnknownRegister(word, n_line);
                    }

                    Register src = kRegisterToNum.at(word);

                    if (!(ss >> word)) {
                        throw CompileError::RRCommandNoModifier(n_line);
                    }

                    int32_t modifier =
                        ParseImmediateOperand(cmd::kModSize, word, n_line);

                    compiled.push_back((command_code << cmd::kCodeShift) |
                                       (recv << cmd::kRecvShift) |
                                       (src << cmd::kSrcShift) |
                                       *reinterpret_cast<uint32_t*>(&modifier));

                    break;
                }

                case RI: {
                    if (!(ss >> word)) {
                        throw CompileError::RICommandNoRegister(n_line);
                    }

                    if (!kRegisterToNum.contains(word)) {
                        throw CompileError::UnknownRegister(word, n_line);
                    }

                    Register recv = kRegisterToNum.at(word);

                    if (!(ss >> word)) {
                        throw CompileError::RICommandNoImmediate(n_line);
                    }

                    int32_t imm =
                        ParseImmediateOperand(cmd::kImmSize, word, n_line);

                    compiled.push_back((command_code << cmd::kCodeShift) |
                                       (recv << cmd::kRecvShift) |
                                       *reinterpret_cast<uint32_t*>(&imm));
                    break;
                }

                case J: {
                    if (!(ss >> word)) {
                        throw CompileError::JCommandNoAddress(n_line);
                    }

                    std::optional<Address> address = ParseAddress(word, n_line);
                    if (address == std::nullopt) {
                        label_usages[word].emplace_back(n_line, n_command);

                        // will be set later during labels substitution
                        address = 0;
                    }

                    compiled.push_back((command_code << cmd::kCodeShift) |
                                       *address);

                    break;
                }

                default: {
                    throw InternalError::UnknownCommandFormat(format, n_line);
                }
            }

            if (ss >> word) {
                CompileError::ExtraWords(format, word, n_line);
            }

            ++n_command;
        }

        for (const auto& [label, usages] : label_usages) {
            if (!label_definitions.contains(label)) {
                // use usages[0], because it is the first occurrence
                // of the label (we store label usages while parsing
                // the file line by line)
                throw CompileError::UndefinedLabel(label, usages[0].first);
            }

            Address address = label_definitions.at(label);
            for (auto [_, n_cmd] : usages) {
                compiled[n_cmd] |= address;
            }
        }
    }

   public:
    void Compile(std::istream& code, std::ostream& bin);

   private:
    std::vector<std::vector<std::string>> words_;

    std::unordered_map<std::string, Address> marks_;
};
}  // namespace karma
