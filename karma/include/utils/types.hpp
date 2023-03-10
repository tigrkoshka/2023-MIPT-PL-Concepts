#pragma once

#include <concepts>     // for [un]signed_integral
#include <cstddef>      // for size_t
#include <sstream>      // for ostringstream
#include <stdexcept>    // for overflow_error
#include <type_traits>  // for make_[un]signed_t
#include <utility>      // for pair

#include "specs/architecture.hpp"

namespace karma::detail::utils::types {

const size_t kByteSize = 8ull;

specs::arch::Double ToDbl(specs::arch::TwoWords);
specs::arch::TwoWords ToUll(specs::arch::Double);

std::pair<specs::arch::Word, specs::arch::Word> Split(specs::arch::TwoWords);
specs::arch::TwoWords Join(specs::arch::Word low, specs::arch::Word high);

/**
 * @brief
 * \b GetSignedValue takes \p bit_size low bits from the provided \p bits
 * and interprets them as a signed integral value of size \p bit_size
 * written in the two's complement representation
 * (see <a href=https://clck.ru/33fHzm>Wiki</a> for details)
 *
 * @tparam
 * Uint is an unsigned integral type large enough to hold
 * the \p bit_size bits of the signed value
 *
 * @param
 * bits are an unsigned integral value of type \p Uint, whose low \p bit_size
 * bits hold the two's complement representation of a signed integral value
 *
 * @param
 * bit_size is the number of low bits of the provided \p bits
 * to be interpreted as a signed integral value in the two's complement
 * representation
 *
 * @return
 * The obtained signed integral value stored to a signed integral type
 * of the same size as \p Uint
 *
 * @note
 * The returned value's bit representation differs from the original low
 * \p bit_size bits of the provided \p bits if:
 * \n\n
 * <tt> bit_size != sizeof(Uint) * kByteSize </tt>
 * \n\n
 * (because the two's complement representation of an integral value is
 * dependant on the size of the integral type representing it)
 * \n \n
 * This behavior is consistent with the aim of this function,
 * which is to capture the signed integral value, and not its representation
 *
 * @throw
 * std::overflow_error
 * if \p bit_size is greater than <tt>sizeof(Uint) * kByteSize</tt>
 *
 * @note
 * This is a reverse function for \b GetUnsignedBits, that is:
 *
 * <tt> ∀ value, bit_size ↪ GetSignedValue(GetUnsignedBits(value, bit_size),
 * bit_size) == value </tt>
 */
template <std::unsigned_integral Uint>
std::make_signed_t<Uint> GetSignedValue(Uint bits, size_t bit_size) {
    using Int            = std::make_signed_t<Uint>;
    size_t orig_bit_size = sizeof(Uint) * kByteSize;

    if (bit_size > orig_bit_size) {
        std::ostringstream ss;
        ss << "overflow: cannot convert a " << orig_bit_size
           << "-bit unsigned value to a " << bit_size << "-bit signed value";

        throw std::overflow_error(ss.str());
    }

    if (bit_size == orig_bit_size) {
        return static_cast<Int>(bits);
    }

    // clear high bits
    Uint shift = static_cast<Uint>(orig_bit_size) - static_cast<Uint>(bit_size);
    bits       = bits << shift >> shift;

    Uint sign_bit = static_cast<Uint>(1) << static_cast<Uint>(bit_size - 1);
    if ((bits & sign_bit) == 0) {
        return static_cast<Int>(bits);
    }

    Uint modulo = (static_cast<Uint>(1) << static_cast<Uint>(bit_size)) - bits;

    return -static_cast<Int>(modulo);
}

/**
 * @brief
 * \b GetUnsignedBits takes a \p value of an integral type and calculates
 * its two's complement representation of size \p bit_size
 * (see <a href=https://clck.ru/33fHzm>Wiki</a> for details)
 *
 * @tparam
 * Int is a signed integral type representing the value
 *
 * @param
 * value is the signed integral value, for which the two's complement
 * representation of size \p bit_size is to be produced
 *
 * @param
 * bit_size is the size of the resulting two's complement
 * representation of the provided \value
 *
 * @return
 * The two's complement representation of size \p bit_size
 * for the provided \p value
 *
 * @note
 * The provided \p value is interpreted exactly as a \a value,
 * i.e. in the mathematical (as opposed to a computational) sense,
 * its \a representation may vary depending on the size of the \p Int type
 *
 * @throw
 * std::overflow_error
 * if \p bit_size is greater than <tt>sizeof(Int) * kByteSize</tt>
 *
 * @note
 * This is a reverse function for \b GetSignedValue, that is:
 *
 * <tt> ∀ bits, bit_size ↪ GetUnsignedBits(GetSignedValue(bits, bit_size),
 * bit_size) == bits </tt>
 */
template <std::signed_integral Int>
std::make_unsigned_t<Int> GetUnsignedBits(Int value, size_t bit_size) {
    using Uint           = std::make_unsigned_t<Int>;
    size_t orig_bit_size = sizeof(Int) * kByteSize;

    if (bit_size > orig_bit_size) {
        std::ostringstream ss;
        ss << "overflow: cannot convert a " << orig_bit_size
           << "-bit signed value to a " << bit_size << "-bit unsigned value";

        throw std::overflow_error(ss.str());
    }

    if (bit_size == orig_bit_size || value >= 0) {
        return static_cast<Uint>(value);
    }

    auto modulo = static_cast<Uint>(-value);

    return (static_cast<Uint>(1) << static_cast<Uint>(bit_size)) - modulo;
}

}  // namespace karma::detail::utils::types