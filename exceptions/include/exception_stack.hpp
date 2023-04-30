#pragma once

#include <csetjmp>          // for jump_buf
#include <optional>         // for optional
#include <source_location>  // for source_location
#include <stack>            // for stack

#include "exception.hpp"

namespace except::detail {

class Node {
   private:
    enum class Status { NO_EXCEPTION, RAISED, HANDLED };

   public:
    Node();

    int* Buff();

    void Raise(Type type, std::source_location);
    bool Handle(std::optional<Type> = std::nullopt);
    void Rethrow() const;
    void Finalize();
    [[nodiscard]] bool IsFinalized() const;

   private:
    std::jmp_buf buf_{};

    Exception exception_;

    volatile Status status_ = Status::NO_EXCEPTION;
    bool finalized_{false};
};

std::optional<Node*> TryGetCurrent();

}  // namespace except::detail
