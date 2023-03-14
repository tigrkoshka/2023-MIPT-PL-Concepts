#pragma once

#include <cstddef>  // for size_t
#include <string>   // for string

#include "specs/architecture.hpp"

namespace karma::detail::specs::exec {

const std::string kDefaultExtension = ".a";

const std::string kIntroString   = "ThisIsKarmaExec";
const size_t kIntroSize          = kIntroString.size() + 1;
constexpr size_t kMetaInfoEndPos = 36;
constexpr size_t kHeaderSize     = 512;
constexpr size_t kCodeSegmentPos = kHeaderSize;
const arch::Word kProcessorID    = 239;

}  // namespace karma::detail::specs::exec
