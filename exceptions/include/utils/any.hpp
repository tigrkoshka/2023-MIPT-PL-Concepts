#pragma once

#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>
#include <any>

#include "utils/concepts.hpp"

namespace except::detail::utils {

class Any {
   private:
    template <typename ValueType>
    friend const ValueType* AnyCast(const Any*) noexcept;

    template <typename ValueType>
    friend ValueType* AnyCast(Any*) noexcept;

   private:
    struct StorageBase {
        virtual ~StorageBase() = default;

        [[nodiscard]] virtual const std::type_info& Type() const noexcept = 0;
        [[nodiscard]] virtual std::unique_ptr<StorageBase> Clone() const  = 0;
    };

    template <std::copy_constructible ValueType>
    struct Storage final : public StorageBase {
        template <typename... Args>
            requires std::constructible_from<ValueType, Args...>
        explicit Storage(Args&&... args)  //
            noexcept(std::is_nothrow_constructible_v<ValueType, Args...>)
            : value(std::forward<Args>(args)...) {}

        [[nodiscard]] const std::type_info& Type() const noexcept override {
            return typeid(ValueType);
        }

        [[nodiscard]] std::unique_ptr<StorageBase> Clone() const override {
            return std::make_unique<Storage<ValueType>>(value);
        }

        ValueType value;
    };

   public:
    // constructors

    constexpr Any() noexcept = default;
    ~Any() noexcept          = default;

    Any(const Any& other) {
        if (other.instance_) {
            instance_ = other.instance_->Clone();
        }
    }

    Any(Any&& other) noexcept
        : instance_(std::move(other.instance_)) {}

    template <typename ValueType>
        requires(!std::same_as<std::decay_t<ValueType>, Any> &&
                 !concepts::InplaceType<ValueType> &&
                 std::copy_constructible<std::decay_t<ValueType>>)
    // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
    Any(ValueType&& value) {  // NOLINT(bugprone-forwarding-reference-overload)
        Emplace<std::decay_t<ValueType>>(std::forward<ValueType>(value));
    }

    template <typename ValueType, typename... Args>
        requires(std::constructible_from<std::decay_t<ValueType>, Args...> &&
                 std::copy_constructible<std::decay_t<ValueType>>)
    explicit Any(std::in_place_type_t<ValueType>, Args&&... args) {
        Emplace<std::decay_t<ValueType>>(std::forward<Args>(args)...);
    }

    template <typename ValueType, typename List, typename... Args>
        requires(std::constructible_from<std::decay_t<ValueType>,
                                         std::initializer_list<List>&,
                                         Args...> &&
                 std::copy_constructible<std::decay_t<ValueType>>)
    explicit Any(std::in_place_type_t<ValueType>,
                 std::initializer_list<List> list,
                 Args&&... args) {
        Emplace<std::decay_t<ValueType>>(list, std::forward<Args>(args)...);
    }

    // assignment operators

    Any& operator=(const Any& rhs) {
        Any(rhs).Swap(*this);
        return *this;
    }

    Any& operator=(Any&& rhs) noexcept {
        Any(std::move(rhs)).Swap(*this);
        return *this;
    }

    template <typename ValueType>
        requires(!std::same_as<std::decay_t<ValueType>, Any> &&
                 std::copy_constructible<std::decay_t<ValueType>>)
    // NOLINTNEXTLINE(fuchsia-overloaded-operator)
    Any& operator=(ValueType&& rhs) {
        Any(std::forward<ValueType>(rhs)).Swap(*this);
        return *this;
    }

    // modifiers

    template <typename ValueType, typename... Args>
        requires(std::constructible_from<std::decay_t<ValueType>, Args...> &&
                 std::copy_constructible<std::decay_t<ValueType>>)
    std::decay_t<ValueType>& Emplace(Args&&... args) {
        auto new_inst = std::make_unique<Storage<std::decay_t<ValueType>>>(
            std::forward<Args>(args)...);

        std::decay_t<ValueType>& value = new_inst->value;
        instance_                      = std::move(new_inst);

        return value;
    }

    template <typename ValueType, typename List, typename... Args>
        requires(std::constructible_from<std::decay_t<ValueType>,
                                         std::initializer_list<List>&,
                                         Args...> &&
                 std::copy_constructible<std::decay_t<ValueType>>)
    std::decay_t<ValueType>& Emplace(std::initializer_list<List> list,
                                     Args&&... args) {
        auto new_inst = std::make_unique<Storage<std::decay_t<ValueType>>>(
            list,
            std::forward<Args>(args)...);

        std::decay_t<ValueType>& value = new_inst->value;
        instance_                      = std::move(new_inst);

        return value;
    }

    void Reset() noexcept {
        instance_.reset();
    }

    void Swap(Any& other) noexcept {
        std::swap(instance_, other.instance_);
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    friend void swap(Any& lhs, Any& rhs) noexcept {
        lhs.Swap(rhs);
    }

    // observers

    [[nodiscard]] bool HasValue() const noexcept {
        return static_cast<bool>(instance_);
    }

    [[nodiscard]] const std::type_info& Type() const noexcept {
        return instance_ ? instance_->Type() : typeid(void);
    }

   private:
    std::unique_ptr<StorageBase> instance_;
};

class BadAnyCast : public std::bad_cast {
   public:
    [[nodiscard]] const char* what() const noexcept override {
        return "bad any cast";
    }
};

// AnyCast

template <typename ValueType>
    requires std::constructible_from<ValueType,
                                     const std::remove_cvref_t<ValueType>&>
ValueType AnyCast(const Any& anything) {
    auto* value = AnyCast<std::remove_cvref_t<ValueType>>(&anything);
    if (value == nullptr) {
        throw BadAnyCast();
    }

    return static_cast<ValueType>(*value);
}

template <typename ValueType>
    requires std::constructible_from<ValueType, std::remove_cvref_t<ValueType>&>
ValueType AnyCast(Any& anything) {
    auto* value = AnyCast<std::remove_cvref_t<ValueType>>(&anything);
    if (value == nullptr) {
        throw BadAnyCast();
    }

    throw BadAnyCast();
}

template <typename ValueType>
    requires std::constructible_from<ValueType, std::remove_cvref_t<ValueType>>
ValueType AnyCast(Any&& anything) {
    auto* value = AnyCast<std::remove_cvref_t<ValueType>>(&anything);
    if (value == nullptr) {
        throw BadAnyCast();
    }

    return static_cast<ValueType>(std::move(*value));
}

template <typename ValueType>
const ValueType* AnyCast(const Any* anything) noexcept {
    if (!anything) {
        return nullptr;
    }

    auto* storage =
        dynamic_cast<Any::Storage<ValueType>*>(anything->instance_.get());
    if (!storage) {
        return nullptr;
    }

    return &storage->value;
}

template <typename ValueType>
ValueType* AnyCast(Any* anything) noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<ValueType*>(
        AnyCast<ValueType>(static_cast<const Any*>(anything)));
}

// MakeAny

template <typename ValueType, typename... Args>
    requires(std::constructible_from<std::decay_t<ValueType>, Args...> &&
             std::copy_constructible<std::decay_t<ValueType>>)
Any MakeAny(Args&&... args) {
    return Any(std::in_place_type<ValueType>, std::forward<Args>(args)...);
}

template <typename ValueType, typename List, typename... Args>
    requires(std::constructible_from<std::decay_t<ValueType>,
                                     std::initializer_list<List>&,
                                     Args...> &&
             std::copy_constructible<std::decay_t<ValueType>>)
Any MakeAny(std::initializer_list<List> list, Args&&... args) {
    return Any(std::in_place_type<ValueType>,
               list,
               std::forward<Args>(args)...);
}

}  // namespace except::detail::utils
