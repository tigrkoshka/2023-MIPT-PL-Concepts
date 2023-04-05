#pragma once

#include <coroutine>  // for coroutine-related symbols
#include <cstddef>    // for ptrdiff_t
#include <exception>  // for exception_ptr
#include <iterator>   // for input_iterator_tag
#include <memory>     // for addressof
#include <utility>    // for exchange

namespace karma::detail::utils {

// TODO: replace with C++23 generator:
// https://clck.ru/33jewB (CppReference)

// This code was developed based on several sources, which are listed below

// Rainer Grimm. Concurrency with Modern C++, pp. 389-391
// Download: https://clck.ru/33jdDF (CloudFlare)

// MSVC:
// https://clck.ru/33jesu (Github)

// CppReference:
// https://clck.ru/33jetK (CppReference)

// StackExchange:
// https://clck.ru/33jetX (StackExchange)

template <typename T>
class Generator {
   public:
    // for the Generator class to be subject to the 'coroutine magic', it should
    // publicly define a struct named exactly promise_type by the C++ standard

    struct promise_type;
    using Handle = std::coroutine_handle<promise_type>;

    // NOLINTNEXTLINE(readability-identifier-naming)
    struct promise_type {
       private:
        friend class Generator<T>;

       public:
        // the function names of this section provide coroutine functionality,
        // are defined by the C++ standard, and therefore cannot be changed
        // see: https://clck.ru/33jeRr (CppReference) for the semantics

        // NOLINTBEGIN(readability-identifier-naming)

        Generator get_return_object() noexcept {
            return Generator{Handle::from_promise(*this)};
        }

        std::suspend_always initial_suspend() noexcept {
            return {};
        }

        std::suspend_always final_suspend() noexcept {
            return {};
        }

        void unhandled_exception() noexcept {
            exception_ = std::current_exception();
        }

        std::suspend_always yield_value(const T& value) noexcept {
            value_ = std::addressof(value);
            return {};
        }

        void return_void() noexcept {}

        // NOLINTEND(readability-identifier-naming)

       private:
        const T* GetValue() noexcept {
            return value_;
        }

        void RethrowIfException() {
            if (exception_) {
                std::rethrow_exception(exception_);
            }
        };

       private:
        const T* value_{nullptr};
        std::exception_ptr exception_{nullptr};
    };

   private:
    class Iterator {
       public:
        // these type definitions allow for user-defined iterators integration
        // with std::ranges, see links for documentation

        // NOLINTBEGIN(readability-identifier-naming)

        // https://clck.ru/33je2h (CppReference)
        using iterator_category = std::input_iterator_tag;

        // https://clck.ru/33je3f (CppReference)
        using difference_type = ptrdiff_t;

        // https://clck.ru/33je5A (CppReference)
        using value_type = T;

        // NOLINTEND(readability-identifier-naming)

       public:
        Iterator() = default;
        explicit Iterator(Handle handle) noexcept
            : handle_(handle) {}

       public:
        // this section defines the iterator behavior, by the C++ standard
        // it should be done by overloading a number of operators

        // NOLINTBEGIN(fuchsia-overloaded-operator)

        Iterator& operator++() {
            handle_.resume();
            if (handle_.done()) {
                std::exchange(handle_, nullptr).promise().RethrowIfException();
            }

            return *this;
        }

        void operator++(int) {
            // This operator meets the requirements of the C++20 input_iterator
            // concept, but not the Cpp17InputIterator requirements
            ++*this;
        }

        [[nodiscard]] bool operator==(const Iterator& other) const noexcept {
            return handle_ == other.handle_;
        }

        // operator != is defined automatically by the compiler since C++20,
        // see https://clck.ru/33ox9D (CppReference)

        [[nodiscard]] const T& operator*() const noexcept {
            return *handle_.promise().GetValue();
        }

        [[nodiscard]] const T* operator->() const noexcept {
            return handle_.promise().GetValue();
        }

        // NOLINTEND(fuchsia-overloaded-operator)

       private:
        Handle handle_{nullptr};
    };

   public:
    // this section defines the functions necessary for the range-based for loop
    // by the C++ standard, so their names cannot be changed

    // NOLINTBEGIN(readability-identifier-naming)

    [[nodiscard]] Iterator begin() {
        if (handle_) {
            handle_.resume();
            if (handle_.done()) {
                std::exchange(handle_, nullptr).promise().RethrowIfException();
                return {};
            }
        }

        return Iterator{handle_};
    }

    [[nodiscard]] Iterator end() noexcept {
        return {};
    }

    // NOLINTEND(readability-identifier-naming)

   public:
    explicit Generator(Handle handle) noexcept
        : handle_(handle) {}

    Generator() = default;
    // NOLINTNEXTLINE(hicpp-use-equals-default,modernize-use-equals-default)
    ~Generator() {
        handle_.destroy();
    }

    // Non-copyable
    Generator(const Generator&)            = delete;
    Generator& operator=(const Generator&) = delete;

    // Movable
    Generator(Generator&& other) noexcept
        : handle_(std::exchange(other.handle_, nullptr)) {}
    Generator& operator=(Generator&& other) noexcept {
        handle_ = std::exchange(other._Coro, nullptr);
        return *this;
    }

   private:
    Handle handle_ = nullptr;
};

}  // namespace karma::detail::utils
