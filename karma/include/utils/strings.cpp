#include "strings.hpp"

#include <cstddef>        // for size_t
#include <string>         // for string
#include <unordered_map>  // for unordered_map

#include "utils/map.hpp"

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
    {'#',  '#' },
};

const std::unordered_map<char, char> kUnescapeSequences =
    map::Revert(kEscapeSequences);

std::string Escape(const std::string& str) {
    std::string res;
    res.reserve(str.size());

    for (char curr : str) {
        if (!kUnescapeSequences.contains(curr)) {
            res += curr;
            continue;
        }

        res += '\\';
        res += kUnescapeSequences.at(curr);
    }

    return res;
}

void Unescape(std::string& str) {
    size_t write_pos = 0;

    for (size_t curr_pos = 0; curr_pos < str.size(); ++curr_pos, ++write_pos) {
        char curr = str[curr_pos];
        if (curr != '\\') {
            str[write_pos] = curr;
            continue;
        }

        ++curr_pos;

        if (curr_pos == str.size()) {
            str[write_pos++] = '\\';
            break;
        }

        curr = str[curr_pos];

        if (!kEscapeSequences.contains(curr)) {
            str[write_pos++] = '\\';
            str[write_pos]   = curr;
            continue;
        }

        str[write_pos] = kEscapeSequences.at(curr);
    }

    str = str.substr(0, write_pos);
}

}  // namespace karma::detail::utils::strings
