#include "exec.hpp"

#include <cstddef>  // for size_t
#include <fstream>  // for ofstream
#include <iomanip>  // for quoted
#include <iosfwd>   // for string
#include <vector>   // for vector

#include "architecture.hpp"
#include "commands.hpp"

namespace karma::detail::specs::exec {

namespace types = arch::types;

////////////////////////////////////////////////////////////////////////////////
///                                 Constants                                ///
////////////////////////////////////////////////////////////////////////////////

const std::string kIntroString = "ThisIsKarmaExec";
const size_t kIntroSize        = kIntroString.size() + 1;
const size_t kCodeSizePos      = 16ull;
const size_t kConstantsSizePos = 20ull;
const size_t kDataSizePos      = 24ull;
const size_t kEntrypointPos    = 28ull;
const size_t kStackInitPos     = 32ull;
const size_t kProcessorIDPos   = 36ull;
const size_t kMetaInfoEndPos   = 40ull;

const size_t kHeaderSize     = 512ll;
const size_t kCodeSegmentPos = kHeaderSize;

static const arch::types::Word kProcessorID = 239ull;

////////////////////////////////////////////////////////////////////////////////
///                                  Errors                                  ///
////////////////////////////////////////////////////////////////////////////////

ExecFileError ExecFileError::FailedToOpen() {
    std::ostringstream ss;
    ss << "failed to open exec file, please check that the path is correct";
    return ExecFileError{ss.str()};
}

ExecFileError ExecFileError::TooSmallForHeader(size_t size) {
    std::ostringstream ss;
    ss << "exec size is " << size << ", which is less than "
       << exec::kHeaderSize << " bytes required for the header";
    return ExecFileError{ss.str()};
}

ExecFileError ExecFileError::InvalidExecSize(size_t exec_size,
                                             size_t code_size,
                                             size_t consts_size,
                                             size_t data_size) {
    std::ostringstream ss;
    ss << "the exec file size (" << exec_size << ") does not equal "
       << exec::kHeaderSize + code_size + consts_size + data_size
       << ", which is the sum of the header size (" << exec::kHeaderSize
       << ") and the total of the sizes of the code segment (" << code_size
       << "), the constants segment (" << consts_size
       << ") and the data segment (" << data_size
       << ") specified in the header";
    return ExecFileError{ss.str()};
}

ExecFileError ExecFileError::InvalidIntroString(const std::string& intro) {
    std::ostringstream ss;
    ss << "expected the welcoming " << std::quoted(exec::kIntroString)
       << " string (and a trailing \\0) as the first " << exec::kIntroSize
       << " bytes, instead got: " << std::quoted(intro);
    return ExecFileError{ss.str()};
}

ExecFileError ExecFileError::InvalidProcessorID(types::Word processor_id) {
    std::ostringstream ss;
    ss << "exec file is built for processor with ID " << processor_id
       << ", current processor ID: " << exec::kProcessorID;
    return ExecFileError{ss.str()};
}

////////////////////////////////////////////////////////////////////////////////
///                                   Write                                  ///
////////////////////////////////////////////////////////////////////////////////

void Write(const Data& data, const std::string& exec_path) {
    std::ofstream binary(exec_path, std::ios::binary | std::ios::trunc);

    // check that the file was found
    if (binary.fail()) {
        throw ExecFileError::FailedToOpen();
    }

    auto write_word = [&binary](types::Word word) {
        binary.write(reinterpret_cast<char*>(&word), 4);
    };

    // intro string
    binary << "ThisIsKarmaExec" << '\0';

    auto get_segment_size =
        [](const std::vector<types::Word>& vec) -> arch::Word {
        return static_cast<types::Word>(vec.size()) *
               static_cast<types::Word>(types::kWordSize);
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
    write_word(static_cast<types::Word>(arch::kMemorySize - 1));

    // target processor ID
    write_word(exec::kProcessorID);

    // empty
    binary << std::string(exec::kCodeSegmentPos - exec::kMetaInfoEndPos, '0');

    // code
    for (cmd::Bin command : data.code) {
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
    if (exec_size < exec::kHeaderSize) {
        throw ExecFileError::TooSmallForHeader(exec_size);
    }

    // reset input position indicator
    binary.seekg(0);

    auto read_word = [&binary]() -> types::Word {
        types::Word word{};
        binary.read(reinterpret_cast<char*>(&word), 4);
        return word;
    };

    // read the first 16 bytes, which should represent
    // the introductory string (including the final '\0')
    std::string intro(exec::kIntroSize, '\0');
    binary.read(intro.data(), static_cast<std::streamsize>(exec::kIntroSize));

    // check that the introductory string is valid
    if (intro != exec::kIntroString) {
        throw ExecFileError::InvalidIntroString(intro);
    }

    Data data{};

    // read the code, constants and data segments sizes (in bytes)
    size_t code_size   = read_word();
    size_t consts_size = read_word();
    size_t data_size   = read_word();

    // check that the exec file size equals the size specified by the header
    if (exec_size != exec::kHeaderSize + code_size + consts_size + data_size) {
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
    types::Word processor_id = read_word();
    if (processor_id != exec::kProcessorID) {
        throw ExecFileError::InvalidProcessorID(processor_id);
    }

    // jump to the code segment
    binary.seekg(exec::kCodeSegmentPos);

    // segments sizes is denoted in bytes, so we need to divide
    // it by types::kWordSize to get the number of machine words

    auto read_segment = [&read_word](size_t byte_size,
                                     std::vector<types::Word>& dst) {
        size_t words_size = byte_size / types::kWordSize;
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
