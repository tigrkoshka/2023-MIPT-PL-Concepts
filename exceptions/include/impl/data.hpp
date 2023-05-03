#pragma once

#include <any>
#include <concepts>
#include <iostream>
#include <memory>
#include <type_traits>
#include <typeinfo>

#include "exceptions/exception.hpp"
#include "utils/concepts.hpp"

namespace except::detail {

struct Data {
   private:
    enum class Operation {
        GET_TYPE_INFO,
        DESTROY,
    };

    struct TypeInfo {
        const std::type_info* type_info;
    };

    template <utils::concepts::Decayed ValueType>
        requires(std::copy_constructible<ValueType> &&
                 std::is_destructible_v<ValueType>)
    struct Manager {
        using StoredType =
            std::conditional_t<std::derived_from<ValueType, Exception>,
                               Exception,
                               ValueType>;

        static void Manage(Operation operation,
                           const Data& data,
                           TypeInfo* ret) {
            switch (operation) {
                case Operation::GET_TYPE_INFO:
                    ret->type_info = &typeid(StoredType);
                    break;

                case Operation::DESTROY:
                    auto ptr = static_cast<StoredType*>(data.data_);
                    if constexpr (std::derived_from<ValueType, Exception>) {
                        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
                        delete dynamic_cast<ValueType*>(ptr);
                    } else {
                        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
                        delete ptr;
                    }
                    break;
            }
        }

        template <typename... Args>
            requires std::constructible_from<ValueType, Args...>
        static void Create(Data& data, Args&&... args) {
            data.data_ = static_cast<StoredType*>(
                // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
                new ValueType(std::forward<Args>(args)...));
        }
    };

    template <utils::concepts::Decayed ValueType>
        requires(std::copy_constructible<ValueType> &&
                 std::is_destructible_v<ValueType>)
    friend struct Manager;

   private:
    template <utils::concepts::Decayed ValueType>
        requires(utils::concepts::NonCVRef<ValueType> &&
                 std::copy_constructible<ValueType> &&
                 std::is_destructible_v<ValueType>)
    [[nodiscard]] ValueType* DoGet() const {
        const std::type_info& stored = Type();

        if constexpr (std::derived_from<ValueType, Exception>) {
            if (stored == typeid(Exception)) {
                return dynamic_cast<ValueType*>(static_cast<Exception*>(data_));
            }
        } else if (stored == typeid(ValueType)) {
            return static_cast<ValueType*>(data_);
        }

        return nullptr;
    }

   public:
    // Constructors

    Data() = default;

    // Non-copyable
    Data(const Data&) = delete;

    // Movable
    Data(Data&& other) noexcept
        : manage_(std::exchange(other.manage_, nullptr)),
          data_(std::exchange(other.data_, nullptr)) {}

    template <typename ValueType>
        requires(!std::same_as<std::decay_t<ValueType>, Data> &&
                 std::copy_constructible<std::decay_t<ValueType>> &&
                 std::is_destructible_v<std::decay_t<ValueType>>)
    // NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
    explicit Data(ValueType&& value)
        : manage_{&Manager<std::decay_t<ValueType>>::Manage} {
        Manager<std::decay_t<ValueType>>::Create(
            *this,
            std::forward<ValueType>(value));
    }

    // Destructor

    ~Data() {
        if (HasValue()) {
            manage_(Operation::DESTROY, *this, nullptr);
        }
    }

    // Assignment operators

    // Non-copyable
    Data& operator=(const Data&) = delete;

    // Movable
    Data& operator=(Data&& other) noexcept {
        manage_ = std::exchange(other.manage_, nullptr);
        data_   = std::exchange(other.data_, nullptr);

        return *this;
    }

    template <typename ValueType>
        requires(!std::same_as<std::decay_t<ValueType>, Data> &&
                 std::copy_constructible<std::decay_t<ValueType>> &&
                 std::is_destructible_v<std::decay_t<ValueType>>)
    // NOLINTNEXTLINE(fuchsia-overloaded-operator)
    Data& operator=(ValueType&& value) {
        *this = Data(std::forward<ValueType>(value));
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

        TypeInfo info;
        manage_(Operation::GET_TYPE_INFO, *this, &info);
        return *info.type_info;
    }

    template <typename ValueType>
        requires(utils::concepts::Decayed<std::remove_cvref_t<ValueType>> &&
                 (std::is_reference_v<ValueType> ||
                  std::copy_constructible<ValueType>) &&
                 std::constructible_from<ValueType,
                                         std::remove_cvref_t<ValueType>&> &&
                 std::is_destructible_v<std::remove_cvref_t<ValueType>>)
    [[nodiscard]] ValueType Get() {
        return static_cast<ValueType>(*DoGet<std::remove_cvref_t<ValueType>>());
    }

    template <typename ValueType>
        requires(utils::concepts::Decayed<std::remove_cvref_t<ValueType>> &&
                 (std::is_reference_v<ValueType> ||
                  std::copy_constructible<ValueType>) &&
                 std::constructible_from<ValueType,
                                         std::remove_cvref_t<ValueType>&> &&
                 std::is_destructible_v<std::remove_cvref_t<ValueType>>)
    [[nodiscard]] bool CanGet() {
        return DoGet<std::remove_cvref_t<ValueType>>() != nullptr;
    }

   private:
    void (*manage_)(Operation, const Data&, TypeInfo*){nullptr};
    void* data_{nullptr};
};

}  // namespace except::detail
