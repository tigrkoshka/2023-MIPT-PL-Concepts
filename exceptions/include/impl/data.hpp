#pragma once

#include <concepts>     // for copy_constructible, ...
#include <type_traits>  // for is_destructible_v, ...
#include <typeinfo>     // for type_info
#include <utility>      // for forward, exchange

#include "types/types.hpp"
#include "utils/concepts.hpp"

namespace except::detail {

struct Data {
   private:
    enum class Operation {
        GET_TYPE_INFO,
        COPY,
        DESTROY,
    };

    struct Return {
        const std::type_info* type_info{nullptr};
        void* copy{nullptr};
    };

    // NOLINTBEGIN(cppcoreguidelines-owning-memory)
    template <utils::concepts::DecayedThrowable Value>
    struct Manager {
        using Stored = std::conditional_t<std::derived_from<Value, Exception>,
                                          Exception,
                                          Value>;

        static void Manage(Operation operation, const Data& data, Return* ret) {
            auto simple_ptr = static_cast<Stored*>(data.data_);
            Value* ptr{nullptr};
            if constexpr (std::same_as<Stored, Exception>) {
                ptr = dynamic_cast<Value*>(simple_ptr);
            } else {
                ptr = simple_ptr;
            }

            switch (operation) {
                case Operation::GET_TYPE_INFO:
                    ret->type_info = &typeid(Stored);
                    break;

                case Operation::COPY:
                    ret->copy = static_cast<Stored*>(new Value(*ptr));
                    break;

                case Operation::DESTROY:
                    delete ptr;
                    break;
            }
        }

        template <typename... Args>
            requires std::constructible_from<Value, Args...>
        static void Create(Data& data, Args&&... args) {
            data.data_ = static_cast<Stored*>(
                new Value(std::forward<Args>(args)...));
        }
    };
    // NOLINTEND(cppcoreguidelines-owning-memory)

    template <utils::concepts::DecayedThrowable Value>
    friend struct Manager;

   private:
    template <utils::concepts::DecayedThrowable Value>
        requires(utils::concepts::NonCVRef<Value>)
    [[nodiscard]] Value* DoGet() const {
        const std::type_info& stored = Type();

        if constexpr (std::derived_from<Value, Exception>) {
            if (stored == typeid(Exception)) {
                return dynamic_cast<Value*>(static_cast<Exception*>(data_));
            }
        } else if (stored == typeid(Value)) {
            return static_cast<Value*>(data_);
        }

        return nullptr;
    }

   public:
    // Constructors

    Data() = default;

    // Copyable
    Data(const Data& other)
        : manage_(other.manage_) {
        Return info{};
        other.manage_(Operation::COPY, other, &info);
        data_ = info.copy;
    }

    // Movable
    Data(Data&& other) noexcept
        : manage_(std::exchange(other.manage_, nullptr)),
          data_(std::exchange(other.data_, nullptr)) {}

    template <utils::concepts::Throwable Value,
              utils::concepts::DecayedThrowable Decayed = std::decay_t<Value>,
              // TODO: this is workaround for a Clang bug, which cannot cope
              //       with the following requires clause correctly,
              //       track a similar issue here: https://clck.ru/34KUsG,
              //       once the bug has been fixed, this template parameter
              //       can be removed in favour of the following requires
              //       clause
              std::enable_if_t<!std::is_same_v<Decayed, Data>, bool> = true>
        requires(!std::same_as<Decayed, Data>)
    // NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
    explicit Data(Value&& value)
        : manage_{&Manager<Decayed>::Manage} {
        Manager<Decayed>::Create(*this, std::forward<Value>(value));
    }

    // Destructor

    ~Data() {
        if (HasValue()) {
            manage_(Operation::DESTROY, *this, nullptr);
        }
    }

    // Assignment operators

    // Copyable
    Data& operator=(const Data& other) {
        if (this == &other) {
            return *this;
        }

        manage_ = other.manage_;

        Return info{};
        other.manage_(Operation::COPY, other, &info);
        data_ = info.copy;

        return *this;
    }

    // Movable
    Data& operator=(Data&& other) noexcept {
        manage_ = std::exchange(other.manage_, nullptr);
        data_   = std::exchange(other.data_, nullptr);

        return *this;
    }

    template <typename Value>
    // NOLINTNEXTLINE(fuchsia-overloaded-operator)
    Data& operator=(Value&& value) {
        *this = Data(std::forward<Value>(value));
        return *this;
    }

    // Observers

    [[nodiscard]] bool HasValue() const noexcept {
        return manage_ != nullptr;
    }

    [[nodiscard]] const std::type_info& Type() const noexcept {
        if (!HasValue()) {
            return typeid(void);
        }

        Return info{};
        manage_(Operation::GET_TYPE_INFO, *this, &info);
        return *info.type_info;
    }

    template <utils::concepts::Catchable ValueType>
    [[nodiscard]] bool CanGet() {
        return DoGet<std::decay_t<ValueType>>() != nullptr;
    }

    template <utils::concepts::Catchable ValueType>
    [[nodiscard]] ValueType Get() {
        return static_cast<ValueType>(*DoGet<std::decay_t<ValueType>>());
    }

   private:
    void (*manage_)(Operation, const Data&, Return*){nullptr};
    void* data_{nullptr};
};

}  // namespace except::detail
