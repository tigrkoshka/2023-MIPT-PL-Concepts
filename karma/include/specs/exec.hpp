#pragma once

#include <cstddef>  // for size_t
#include <string>   // for string

#include "architecture.hpp"

namespace karma::detail::specs::exec {

const std::string kDefaultExtension = ".a";

const std::string kIntroString   = "ThisIsKarmaExec";
const size_t kIntroSize          = kIntroString.size() + 1;
constexpr size_t kMetaInfoEndPos = 36ull;
constexpr size_t kHeaderSize     = 512ll;
constexpr size_t kCodeSegmentPos = kHeaderSize;

const arch::Word kProcessorID = 239ull;

}  // namespace karma::detail::specs::exec
