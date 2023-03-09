#pragma once

#include <cstddef>    // for size_t
#include <stdexcept>  // for runtime_error
#include <string>     // for string
#include <vector>     // for vector

#include "architecture.hpp"
#include "commands.hpp"

namespace karma::detail::specs::exec {

const std::string kDefaultExtension = ".a";

const std::string kIntroString = "ThisIsKarmaExec";
const size_t kIntroSize        = kIntroString.size() + 1;
const size_t kMetaInfoEndPos   = 40ull;
const size_t kHeaderSize       = 512ll;
const size_t kCodeSegmentPos   = kHeaderSize;

static const arch::Word kProcessorID = 239ull;

struct Data {
    specs::arch::Address entrypoint;
    specs::arch::Address initial_stack;

    std::vector<cmd::Bin> code;
    std::vector<arch::Word> constants;
};

void Write(const Data& data, const std::string& exec_path);
Data Read(const std::string& exec_path);

}  // namespace karma::detail::specs::exec
