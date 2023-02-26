#pragma once

#include <cstddef>  // for size_t
#include <string>   // for string

#include "architecture.hpp"

namespace karma::detail::specs::exec {

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

}  // namespace karma::detail::specs::exec
