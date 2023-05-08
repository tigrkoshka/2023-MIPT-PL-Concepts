#pragma once

#include <except>
#include <string>   // for string
#include <utility>  // for move

namespace except::test::impl {

struct CustomExceptionParent : Exception {
    ~CustomExceptionParent() override = default;

    explicit CustomExceptionParent(std::string message)
        : message(std::move(message)) {}

    std::string message;
};

struct CustomExceptionChild : CustomExceptionParent {
    ~CustomExceptionChild() override = default;

    explicit CustomExceptionChild(std::string message)
        : CustomExceptionParent(std::move(message)) {}
};

struct CustomExceptionOther : RuntimeError {
    ~CustomExceptionOther() override = default;
};

}  // namespace except::test::impl
