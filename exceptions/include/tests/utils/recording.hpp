#pragma once

#include <cstddef>  // for destructors
#include <vector>   // for vector

namespace except::test::impl {

struct Recording {
    Recording();
    ~Recording();

    static void Reset();

    static std::vector<size_t> destructors;
    static size_t index;

   private:
    size_t index_;
};

}  // namespace except::test::impl
