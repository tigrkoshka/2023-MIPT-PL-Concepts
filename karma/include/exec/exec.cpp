#include "exec.hpp"

#include <cstddef>  // for size_t
#include <fstream>  // for ofstream
#include <iosfwd>   // for ostringstream
#include <sstream>  // for ostringstream
#include <string>   // for string
#include <vector>   // for vector

#include "errors.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "specs/exec.hpp"

namespace karma::detail::exec {

using errors::exec::ExecFileError;

namespace arch = specs::arch;

namespace cmd = specs::cmd;

namespace exec = specs::exec;

////////////////////////////////////////////////////////////////////////////////
///                                   Write                                  ///
////////////////////////////////////////////////////////////////////////////////

void Write(const Data& data, const std::string& exec_path) {
    std::ofstream binary(exec_path, std::ios::binary | std::ios::trunc);

    // check that the file was found
    if (binary.fail()) {
        throw ExecFileError::FailedToOpen(exec_path);
    }

    auto write_word = [&binary](arch::Word word) {
        binary.write(reinterpret_cast<char*>(&word), 4);
    };

    // intro string
    binary << "ThisIsKarmaExec" << '\0';

    auto get_segment_size =
        [](const std::vector<arch::Word>& vec) -> arch::Word {
        return static_cast<arch::Word>(vec.size()) *
               static_cast<arch::Word>(arch::kWordSize);
    };

    // code size (in bytes)
    write_word(get_segment_size(data.code));

    // constants size (in bytes)
    write_word(get_segment_size(data.constants));

    // entrypoint address
    write_word(data.entrypoint);

    // initial stack pointer
    write_word(static_cast<arch::Word>(arch::kMemorySize - 1));

    // target processor ID
    write_word(exec::kProcessorID);

    // empty
    binary << std::string(exec::kCodeSegmentPos - exec::kMetaInfoEndPos, '0');

    // code
    for (cmd::Bin command : data.code) {
        write_word(command);
    }

    // constants
    for (cmd::Bin command : data.constants) {
        write_word(command);
    }
}

////////////////////////////////////////////////////////////////////////////////
///                                   Read                                   ///
////////////////////////////////////////////////////////////////////////////////

Data Read(const std::string& exec_path) {
    // set the input position indicator to the end (using std::ios::ate)
    // after opening the file to get the size of the exec file,
    // which is used later to verify that the exec is not malformed
    std::ifstream binary(exec_path, std::ios::binary | std::ios::ate);

    // check that the file was found
    if (binary.fail()) {
        throw ExecFileError::FailedToOpen(exec_path);
    }

    // check that the exec size is not too small to contain a valid header
    auto exec_size = static_cast<size_t>(binary.tellg());
    if (exec_size < exec::kHeaderSize) {
        throw ExecFileError::TooSmallForHeader(exec_size, exec_path);
    }

    // check that the combined code and constants segments sizes
    // are not too big to fit into memory
    if (exec_size - exec::kHeaderSize > arch::kMemorySize) {
        throw ExecFileError::TooBigForMemory(exec_size - exec::kHeaderSize,
                                             exec_path);
    }

    // reset input position indicator
    binary.seekg(0);

    auto read_word = [&binary]() -> arch::Word {
        arch::Word word{};
        binary.read(reinterpret_cast<char*>(&word), 4);
        return word;
    };

    // read the first 16 bytes, which should represent
    // the introductory string (including the final '\0')
    std::string intro(exec::kIntroSize, '\0');
    binary.read(intro.data(), static_cast<std::streamsize>(exec::kIntroSize));

    // check that the introductory string is valid
    if (intro.back() != '\0') {
        intro.resize(intro.size() - 1);
        throw ExecFileError::NoTrailingZeroInIntro(intro, exec_path);
    }

    intro.resize(intro.size() - 1);
    if (intro != exec::kIntroString) {
        throw ExecFileError::InvalidIntroString(intro, exec_path);
    }

    Data data{};

    // read the code and constants segments sizes (in bytes)
    size_t code_size   = read_word();
    size_t consts_size = read_word();

    // check that the exec file size equals the size specified by the header
    if (exec_size != exec::kHeaderSize + code_size + consts_size) {
        throw ExecFileError::InvalidExecSize(exec_size,
                                             code_size,
                                             consts_size,
                                             exec_path);
    }

    // read the address of the first instruction
    data.entrypoint = read_word();

    // read the initial stack pointer value
    data.initial_stack = read_word();

    // read the target processor ID
    arch::Word processor_id = read_word();
    if (processor_id != exec::kProcessorID) {
        throw ExecFileError::InvalidProcessorID(processor_id, exec_path);
    }

    // jump to the code segment
    binary.seekg(exec::kCodeSegmentPos);

    // segments sizes is denoted in bytes, so we need to divide
    // it by arch::kWordSize to get the number of machine words

    auto read_segment = [&read_word](size_t byte_size,
                                     std::vector<arch::Word>& dst) {
        size_t words_size = byte_size / arch::kWordSize;
        dst.reserve(words_size);

        for (size_t i = 0; i < words_size; ++i) {
            dst.push_back(read_word());
        }
    };

    // read code
    read_segment(code_size, data.code);

    // read constants
    read_segment(consts_size, data.constants);

    return data;
}

}  // namespace karma::detail::exec