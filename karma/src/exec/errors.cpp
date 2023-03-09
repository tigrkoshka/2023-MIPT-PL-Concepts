#include "errors.hpp"

#include <iomanip>  // for quoted
#include <sstream>  // for ostringstream

#include "../specs/architecture.hpp"
#include "../specs/exec.hpp"

namespace karma::errors::exec {

namespace arch = detail::specs::arch;

namespace exec = detail::specs::exec;

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

ExecFileError ExecFileError::TooBigForMemory(size_t size) {
    std::ostringstream ss;
    ss << "the combined size of the code and constants segments is " << size
       << ", which is greater than the memory size " << arch::kMemorySize
       << ", so the code and the constants do not fit into the memory";
    return ExecFileError{ss.str()};
}

ExecFileError ExecFileError::InvalidExecSize(size_t exec_size,
                                             size_t code_size,
                                             size_t consts_size) {
    std::ostringstream ss;
    ss << "the exec file size (" << exec_size << ") does not equal "
       << exec::kHeaderSize + code_size + consts_size
       << ", which is the sum of the header size (" << exec::kHeaderSize
       << ") and the total of the sizes of the code segment (" << code_size
       << ") and the constants segment (" << consts_size
       << ") specified in the header";
    return ExecFileError{ss.str()};
}

ExecFileError ExecFileError::NoTrailingZeroInIntro(const std::string& intro) {
    std::ostringstream ss;
    ss << "expected the welcoming " << std::quoted(exec::kIntroString)
       << R"( string (and a trailing '\0') as the first )" << exec::kIntroSize
       << R"( bytes, but the first 16 bytes do not end with a '\0': )" << intro;
    return ExecFileError{ss.str()};
}

ExecFileError ExecFileError::InvalidIntroString(const std::string& intro) {
    std::ostringstream ss;
    ss << "expected the welcoming " << std::quoted(exec::kIntroString)
       << R"( string and a trailing '\0' as the first )" << exec::kIntroSize
       << " bytes, instead got: " << std::quoted(intro)
       << R"(" (with a trailing '\0'))";
    return ExecFileError{ss.str()};
}

ExecFileError ExecFileError::InvalidProcessorID(arch::Word processor_id) {
    std::ostringstream ss;
    ss << "exec file is built for processor with ID " << processor_id
       << ", current processor ID: " << exec::kProcessorID;
    return ExecFileError{ss.str()};
}

}  // namespace karma::errors::exec