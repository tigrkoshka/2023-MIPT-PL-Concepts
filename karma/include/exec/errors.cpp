#include "errors.hpp"

#include <iomanip>  // for quoted
#include <sstream>  // for ostringstream

#include "specs/architecture.hpp"
#include "specs/exec.hpp"

namespace karma::errors::exec {

using FE = ExecFileError;

namespace arch = detail::specs::arch;

namespace exec = detail::specs::exec;

FE FE::FailedToOpen(const std::string& path) {
    std::ostringstream ss;
    ss << "failed to open exec file";
    return {ss.str(), path};
}

FE FE::TooSmallForHeader(size_t size, const std::string& path) {
    std::ostringstream ss;
    ss << "exec size is " << size << ", which is less than "
       << exec::kHeaderSize << " bytes required for the header";
    return {ss.str(), path};
}

FE FE::TooBigForMemory(size_t size, const std::string& path) {
    std::ostringstream ss;
    ss << "the combined size of the code and constants segments is " << size
       << ", which is greater than the memory size " << arch::kMemorySize
       << ", so the code and the constants do not fit into the memory";
    return {ss.str(), path};
}

FE FE::InvalidExecSize(size_t exec_size,
                       size_t code_size,
                       size_t consts_size,
                       const std::string& path) {
    std::ostringstream ss;
    ss << "the exec file size (" << exec_size << ") does not equal "
       << exec::kHeaderSize + code_size + consts_size
       << ", which is the sum of the header size (" << exec::kHeaderSize
       << ") and the total of the sizes of the code segment (" << code_size
       << ") and the constants segment (" << consts_size
       << ") specified in the header";
    return {ss.str(), path};
}

FE FE::NoTrailingZeroInIntro(const std::string& intro,
                             const std::string& path) {
    std::ostringstream ss;
    ss << "expected the welcoming " << std::quoted(exec::kIntroString)
       << R"( string (and a trailing '\0') as the first )" << exec::kIntroSize
       << R"( bytes, but the first 16 bytes do not end with a '\0': )" << intro;
    return {ss.str(), path};
}

FE FE::InvalidIntroString(const std::string& intro, const std::string& path) {
    std::ostringstream ss;
    ss << "expected the welcoming " << std::quoted(exec::kIntroString)
       << R"( string and a trailing '\0' as the first )" << exec::kIntroSize
       << " bytes, instead got: " << std::quoted(intro)
       << R"(" (with a trailing '\0'))";
    return {ss.str(), path};
}

FE FE::InvalidProcessorID(arch::Word processor_id, const std::string& path) {
    std::ostringstream ss;
    ss << "exec file is built for processor with ID " << processor_id
       << ", current processor ID: " << exec::kProcessorID;
    return {ss.str(), path};
}

}  // namespace karma::errors::exec