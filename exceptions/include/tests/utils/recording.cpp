#include "recording.hpp"

namespace except::test::impl {

Recording::Recording()
    : index_(index++) {}

// NOLINTNEXTLINE(bugprone-exception-escape)
Recording::~Recording() noexcept {
    destructors.push_back(index_);
}

void Recording::Reset() {
    destructors = {};
    index = 0;
}

std::vector<size_t> Recording::destructors{};
size_t Recording::index = 0;

}  // namespace except::test::impl
