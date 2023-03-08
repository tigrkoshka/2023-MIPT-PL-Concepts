#include "strings.hpp"

#include <cstddef>        // for size_t
#include <string>         // for string
#include <unordered_map>  // for unordered_map

namespace karma::detail::utils::strings {

const std::string kWhitespaces = " \t\n\v\f\r";

void TrimSpaces(std::string& str) {
    const size_t start = str.find_first_not_of(kWhitespaces);
    if (start == std::string::npos) {
        str = "";
        return;
    }

    const size_t end = str.find_last_not_of(kWhitespaces);

    str = str.substr(start, end - start + 1);
}

const std::unordered_map<char, char> kEscapeSequences{
    {'\'', '\''},
    {'\"', '\"'},
    {'?',  '\?'},
    {'\\', '\\'},
    {'a',  '\a'},
    {'b',  '\b'},
    {'f',  '\f'},
    {'n',  '\n'},
    {'r',  '\r'},
    {'t',  '\t'},
    {'v',  '\v'},
};

void Unescape(std::string& str) {
    size_t curr_pos = 0;

    for (auto it = str.begin(); it != str.end(); ++it, ++curr_pos) {
        char curr = *it;
        if (curr != '\\') {
            str[curr_pos] = curr;
            continue;
        }

        ++it;

        if (it == str.end()) {
            str[curr_pos] = '\\';
            break;
        }

        curr = *it;

        if (!kEscapeSequences.contains(curr)) {
            str[curr_pos++] = '\\';
            str[curr_pos]   = curr;
            continue;
        }

        str[curr_pos] = kEscapeSequences.at(curr);
    }

    str = str.substr(0, curr_pos);
}

}  // namespace karma::detail::utils::strings
