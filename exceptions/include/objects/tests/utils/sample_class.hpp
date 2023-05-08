#pragma once

#include <cstddef>
#include <string>
#include <utility>

#include "objects/tests/utils/stats.hpp"

#define EXCEPT_OBJECTS_TEST_CLASS_CONTENT(ClassName)                        \
    /* Constructors */                                                      \
                                                                            \
    explicit(Explicit) ClassName() noexcept(Noexcept) {                     \
        ++stats.ctor.default_;                                              \
    }                                                                       \
                                                                            \
    explicit(Explicit) ClassName(const std::string& str) noexcept(Noexcept) \
        : index(str.size()) {                                               \
        ++stats.ctor.single_argument;                                       \
    }                                                                       \
                                                                            \
    explicit(Explicit)                                                      \
        ClassName(size_t idx, const std::string& str) noexcept(Noexcept)    \
        : index(idx + str.size()) {                                         \
        ++stats.ctor.multi_argument;                                        \
    }                                                                       \
                                                                            \
    explicit(Explicit) ClassName(const ClassName& other) noexcept(Noexcept) \
        : index(other.index) {                                              \
        ++stats.ctor.copy;                                                  \
    }                                                                       \
                                                                            \
    explicit(Explicit) ClassName(ClassName&& other) noexcept(Noexcept)      \
        : index(std::exchange(other.index, 0)) {                            \
        ++stats.ctor.move;                                                  \
    }                                                                       \
                                                                            \
    /* Destructor */                                                        \
                                                                            \
    ~ClassName() noexcept(Noexcept) {                                       \
        ++stats.dtor;                                                       \
    }                                                                       \
                                                                            \
    /* Assignments */                                                       \
                                                                            \
    ClassName& operator=(const ClassName& other) noexcept(Noexcept) {       \
        if (this == &other) {                                               \
            return *this;                                                   \
        }                                                                   \
                                                                            \
        ++stats.assign.copy;                                                \
        index = other.index;                                                \
        return *this;                                                       \
    };                                                                      \
                                                                            \
    ClassName& operator=(ClassName&& other) noexcept(Noexcept) {            \
        ++stats.assign.move;                                                \
        index = std::exchange(other.index, 0);                              \
        return *this;                                                       \
    };                                                                      \
                                                                            \
    /* Conversion */                                                        \
                                                                            \
    explicit(Explicit) operator bool() const noexcept(Noexcept) {           \
        return index > 0;                                                   \
    }                                                                       \
                                                                            \
    /* Operators */                                                         \
                                                                            \
    std::strong_ordering operator<=>(const ClassName&)                      \
        const noexcept(Noexcept) = default;                                 \
                                                                            \
    ClassName& operator++() noexcept(Noexcept) {                            \
        ++index;                                                            \
        return *this;                                                       \
    }                                                                       \
                                                                            \
    ClassName operator++(int)& noexcept(                                    \
        Noexcept) { /* NOLINT(performance-*) */                             \
        ClassName old = *this;                                              \
        operator++();                                                       \
        return old;                                                         \
    }                                                                       \
                                                                            \
    /* Methods */                                                           \
                                                                            \
    void IncrementIndex() noexcept(Noexcept) {                              \
        ++index;                                                            \
    }                                                                       \
                                                                            \
    [[nodiscard]] size_t Index() const noexcept(Noexcept) {                 \
        return index;                                                       \
    }                                                                       \
                                                                            \
    /* Fields */                                                            \
                                                                            \
    size_t index{0};                                                        \
    static Stats stats;

namespace except::test::objects {

template <bool Explicit, bool Noexcept>
struct Simple {
    EXCEPT_OBJECTS_TEST_CLASS_CONTENT(Simple)
};

template <bool Explicit, bool Noexcept>
Stats Simple<Explicit, Noexcept>::stats;

using SimplePlain    = Simple<false, false>;
using SimpleExplicit = Simple<true, false>;
using SimpleNoexcept = Simple<false, true>;

template <bool Explicit, bool Noexcept>
struct Final final {
    EXCEPT_OBJECTS_TEST_CLASS_CONTENT(Final)
};

template <bool Explicit, bool Noexcept>
Stats Final<Explicit, Noexcept>::stats;

using FinalPlain    = Final<false, false>;
using FinalExplicit = Final<true, false>;
using FinalNoexcept = Final<false, true>;

}  // namespace except::test::objects

#undef EXCEPT_OBJECTS_TEST_CLASS_CONTENT
