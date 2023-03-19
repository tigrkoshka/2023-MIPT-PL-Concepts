#pragma once

#include <cstddef>  // for size_t
#include <string>   // for string

namespace karma::detail::utils::strings {

/**
 * @brief
 * Deletes the ASCII whitespace characters from the beginning
 * and the end of a string.
 *
 * \note
 * The deletion is performed inplace, i.e. the provided string
 * is modified and no copies occur.
 */
void TrimSpaces(std::string& str);

/**
 * @brief
 * Replaces certain special characters in the specified string
 * with respective escape sequences.\n
 * The list of special characters consists of '#' (the hash sign), which is
 * replaced with a "\\#" sequence and the characters defined in the
 * <a href=https://clck.ru/33oJkT>Simple escape sequences</a> table
 * of the C++ standard.
 *
 * @param
 * str is the string in which the special characters are to be replaced
 * with escape sequences
 *
 * @return
 * The specified string with the special characters replaced
 * with respective escape sequences.
 *
 * @note
 * This is a reverse function for \b Unescape, that is:\n
 * \n
 * For all possible values of \p str after:\n
 * \n
 * <tt>
 * Unescape(str);\n
 * str = Escape(str);\n
 * </tt>
 * \n
 * the \p str value does not change
 */
std::string Escape(const std::string& str);

/**
 * @brief
 * Replaces certain escape sequences in the specified string
 * with respective special character.\n
 * The list of escape sequences consists of "\\#", which is
 * replaced with a '#' (the hash sign) and the escape sequences defined in the
 * <a href=https://clck.ru/33oJkT>Simple escape sequences</a> table
 * of the C++ standard.
 *
 * \note
 * The escape sequences are replaced inplace, i.e. the provided string
 * is modified and no copies occur.
 *
 * @param
 * str is the string in which the escape sequences are to be replaced
 * with special characters
 *
 * @note
 * This is a reverse function for \b Escape, that is:\n
 * \n
 * For all possible values of \p str after:\n
 * \n
 * <tt>
 * std::string new_str = Escape(str);\n
 * Unescape(new_str);\n
 * </tt>
 * \n
 * the <tt>new_str</tt> is equal to \p str
 */
void Unescape(std::string& str);

}  // namespace karma::detail::utils::strings
