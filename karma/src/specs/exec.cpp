#include "exec.hpp"

#include <cstddef>  // for size_t
#include <fstream>  // for ofstream
#include <iomanip>  // for quoted
#include <iosfwd>   // for string, ostringstream
#include <sstream>  // for ostringstream
#include <vector>   // for vector

#include "../errors/exec_errors.hpp"
#include "architecture.hpp"
#include "commands.hpp"

namespace karma::detail::specs::exec {

using errors::exec::ExecFileError;

////////////////////////////////////////////////////////////////////////////////
///                                   Write                                  ///
////////////////////////////////////////////////////////////////////////////////

void Write(const Data& data, const std::string& exec_path) {
    std::ofstream binary(exec_path, std::ios::binary | std::ios::trunc);

    // check that the file was found
    if (binary.fail()) {
        throw ExecFileError::FailedToOpen();
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

    // const size (in bytes)
    write_word(get_segment_size(data.constants));

    // data size (in bytes)
    write_word(get_segment_size(data.data));

    // entrypoint address
    write_word(data.entrypoint);

    // initial stack pointer
    write_word(static_cast<arch::Word>(arch::kMemorySize - 1));

    // target processor ID
    write_word(kProcessorID);

    // empty
    binary << std::string(kCodeSegmentPos - kMetaInfoEndPos, '0');

    // code
    for (cmd::Bin command : data.code) {
        write_word(command);
    }

    // constants
    for (cmd::Bin command : data.constants) {
        write_word(command);
    }

    // data
    for (cmd::Bin command : data.data) {
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
        throw ExecFileError::FailedToOpen();
    }

    // check that the exec size is not too small to contain a valid header
    auto exec_size = static_cast<size_t>(binary.tellg());
    if (exec_size < kHeaderSize) {
        throw ExecFileError::TooSmallForHeader(exec_size);
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
    std::string intro(kIntroSize, '\0');
    binary.read(intro.data(), static_cast<std::streamsize>(kIntroSize));

    // check that the introductory string is valid
    if (intro.back() != '\0') {
        intro.resize(intro.size() - 1);
        throw ExecFileError::NoTrailingZeroInIntro(intro);
    }

    intro.resize(intro.size() - 1);
    if (intro != kIntroString) {
        throw ExecFileError::InvalidIntroString(intro);
    }

    Data data{};

    // read the code, constants and data segments sizes (in bytes)
    size_t code_size   = read_word();
    size_t consts_size = read_word();
    size_t data_size   = read_word();

    // check that the exec file size equals the size specified by the header
    if (exec_size != kHeaderSize + code_size + consts_size + data_size) {
        throw ExecFileError::InvalidExecSize(exec_size,
                                             code_size,
                                             consts_size,
                                             data_size);
    }

    // read the address of the first instruction
    data.entrypoint = read_word();

    // read the initial stack pointer value
    data.initial_stack = read_word();

    // read the target processor ID
    arch::Word processor_id = read_word();
    if (processor_id != kProcessorID) {
        throw ExecFileError::InvalidProcessorID(processor_id);
    }

    // jump to the code segment
    binary.seekg(kCodeSegmentPos);

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

    // read data
    read_segment(data_size, data.data);

    return data;
}

}  // namespace karma::detail::specs::exec
