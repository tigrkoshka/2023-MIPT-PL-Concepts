#pragma once

#include <csetjmp>   // for jump_buf
#include <optional>  // for optional
#include <stack>     // for stack

#include "exception.hpp"

namespace except::detail {

class Node {
   private:
    enum class Status { NO_EXCEPTION, RAISED, HANDLED };

   public:
    Node();

    int* Buff();

    void Raise(Type type, const char* file, size_t line);
    bool Handle(std::optional<Type> = std::nullopt);
    void Finalize();
    [[nodiscard]] bool IsFinalized() const;

    static std::optional<Node*> TryGetCurrent();

   private:
    std::jmp_buf buf_{};

    Exception exception_;

    volatile Status status_ = Status::NO_EXCEPTION;
    bool finalized_{false};
};

}  // namespace except::detail
