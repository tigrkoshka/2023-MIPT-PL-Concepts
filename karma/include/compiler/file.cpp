#include "file.hpp"

#include <algorithm>   // for min
#include <cstddef>     // for size_t
#include <filesystem>  // for path
#include <fstream>     // for ifstream
#include <iterator>    // for ostream_iterator
#include <ranges>      // for ranges, views
#include <string>      // for string
#include <utility>     // for move

#include "compiler/compiler.hpp"
#include "specs/syntax.hpp"
#include "utils/generator.hpp"
#include "utils/strings.hpp"

namespace karma {

namespace utils  = detail::utils;
namespace syntax = detail::specs::syntax;

void Compiler::File::TrimComment(std::string& line) {
    size_t curr_start_pos = 0;

    size_t comment_start{};
    while (true) {
        comment_start = line.find(syntax::kCommentSep, curr_start_pos);

        if (comment_start == curr_start_pos ||
            comment_start == std::string::npos) {
            break;
        }

        if (line[comment_start - 1] == syntax::kDisableCommentSep) {
            curr_start_pos = comment_start + 1;
            continue;
        }

        break;
    }

    line.resize(std::min(comment_start, line.size()));
}

utils::Generator<const Compiler::File*> Compiler::File::ToRoot() const {
    for (const File* curr = this; curr != nullptr; curr = curr->parent_) {
        co_yield curr;
    }
};

void Compiler::File::Open() {
    if (stream_.is_open()) {
        throw InternalError::RepeatedOpenFile(path_);
    }

    stream_ = std::ifstream(path_);

    if (stream_.fail()) {
        throw CompileError::FailedToOpen(path_);
    }

    line_ = 0;
}

void Compiler::File::Close() {
    if (!stream_.is_open()) {
        throw InternalError::CloseUnopenedFile(path_);
    }

    stream_.close();
}

bool Compiler::File::NextLine() {
    std::string line;
    if (!std::getline(stream_, line)) {
        return false;
    }

    TrimComment(line);

    // C++20 feature for moving a string into the istringstream,
    // may not be supported by some STL versions, but is supported by GNU
    curr_line_ = std::istringstream(std::move(line));
    ++line_;

    return true;
}

bool Compiler::File::GetLine(std::string& line) {
    if (!std::getline(curr_line_, line)) {
        return false;
    }

    utils::strings::TrimSpaces(line);

    return !line.empty();
}

bool Compiler::File::GetToken(std::string& token) {
    return static_cast<bool>(curr_line_ >> token);
}

std::string Compiler::File::Where() const {
    const std::string sep = "\n included from ";

    std::ostringstream where;
    where << "at line " << LineNum() << "\n"
          << std::string(sep.size() - 4, ' ') << "in ";

    auto files    = ToRoot();
    auto get_path = [](const File* file) -> std::string {
        return file->Path();
    };

    auto pipeline = files | std::views::transform(get_path);

    // TODO: there is an std::experimental::ostream_joiner
    //       currently defines in <experimental/iterator>
    //       that does exactly what is needed, but we do not
    //       want to use experimental features, so here is a workaround

    std::ostream_iterator<std::string> dst{where, sep.data()};
    std::ranges::copy(pipeline, dst);

    // remove the trailing separator
    std::string res = where.str();
    res.resize(res.size() - sep.size());

    return res;
}

size_t Compiler::File::LineNum() const {
    return line_;
}

const std::filesystem::path& Compiler::File::Path() const {
    return path_;
}

}  // namespace karma