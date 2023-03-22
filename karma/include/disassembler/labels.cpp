#include "labels.hpp"

#include <optional>  // for optional, nullopt
#include <set>       // for set
#include <string>    // for string, to_string

#include "disassembler/disassembler.hpp"
#include "disassembler/errors.hpp"
#include "exec/exec.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"

namespace karma {

namespace arch = detail::specs::arch;
namespace cmd  = detail::specs::cmd;

std::string Disassembler::Labels::RecordConstantLabel(arch::Address address) {
    std::string label =
        kConstantLabelPrefix + std::to_string(constant_labels_.size() + 1);

    constant_labels_[address] = label;

    return label;
}

void Disassembler::Labels::PrepareCommandLabels(const Exec::Data& data) {
    auto code_end_address = static_cast<arch::Address>(data.code.size());

    // use ordered set for the resulting labels indices
    // to be the same order as they appear in the code
    std::set<arch::Address> command_labels_addresses;
    for (cmd::Bin command : data.code) {
        cmd::Code code = cmd::GetCode(command);

        if (!cmd::kCodeToFormat.contains(code)) {
            throw DisassembleError::UnknownCommand(code);
        }

        switch (cmd::kCodeToFormat.at(code)) {
            case cmd::RM: {
                arch::Address addr = cmd::parse::RM(command).addr;

                if (addr < code_end_address) {
                    command_labels_addresses.insert(addr);
                }
                break;
            }

            case cmd::J: {
                arch::Address addr = cmd::parse::J(command).addr;

                // prevent unnecessary labels in RET and
                if (!cmd::kJIgnoreAddress.contains(code) &&
                    addr < code_end_address) {
                    command_labels_addresses.insert(addr);
                }
                break;
            }

            default:
                // only RM and J format commands can use labels
                continue;
        }
    }

    command_labels_[data.entrypoint] = kMainLabel;

    for (arch::Address address : command_labels_addresses) {
        // the entrypoint has a special label
        if (address == data.entrypoint) {
            continue;
        }

        // indexing labels with command_labels_.size() will start with 1
        // since we have already written the entrypoint label
        command_labels_[address] =
            kCommandLabelPrefix + std::to_string(command_labels_.size());
    }
}

std::string Disassembler::Labels::MainLabel() {
    return kMainLabel;
}

std::optional<std::string> Disassembler::Labels::TryGetLabel(
    arch::Address address) const {
    if (constant_labels_.contains(address)) {
        return constant_labels_.at(address);
    }

    if (command_labels_.contains(address)) {
        return command_labels_.at(address);
    }

    return std::nullopt;
}

const std::string Disassembler::Labels::kConstantLabelPrefix = ".constant_";
const std::string Disassembler::Labels::kCommandLabelPrefix  = "command_label_";
const std::string Disassembler::Labels::kMainLabel           = "main";

}  // namespace karma