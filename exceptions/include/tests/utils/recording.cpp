#include "recording.hpp"

namespace except::test::impl {

Recording::Recording()
    : index_(index++) {}

Recording::~Recording() {
    destructors.push_back(index_);
}

void Recording::Reset() {
    destructors = {};
    index = 0;
}

std::vector<size_t> Recording::destructors{};
size_t Recording::index = 0;

}  // namespace except::test::impl
