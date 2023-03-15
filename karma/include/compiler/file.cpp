#include "file.hpp"

#include <algorithm>   // for min
#include <cstddef>     // for size_t
#include <filesystem>  // for path
#include <fstream>     // for ifstream
#include <iterator>    // for ostream_iterator
#include <ranges>      // for ranges, views
#include <stack>       // for stack
#include <string>      // for string
#include <utility>     // for move

#include "compiler/compiler.hpp"
#include "specs/syntax.hpp"
#include "utils/strings.hpp"

namespace karma {

using karma::errors::compiler::InternalError;
using karma::errors::compiler::CompileError;

namespace utils = karma::detail::utils;

namespace syntax = karma::detail::specs::syntax;

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

utils::Generator<const Compiler::File*> Compiler::File::FromRoot() const {
    std::stack<const File*> stack;
    for (const File* curr = this; curr != nullptr; curr = curr->parent_) {
        stack.push(curr);
    }

    while (!stack.empty()) {
        co_yield stack.top();
        stack.pop();
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

    line_ = 1;
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
    std::ostringstream where;
    where << "at line " << LineNum() << " in ";

    auto x = FromRoot();

    auto get_path = [](const File* file) -> std::string {
        return file->Path();
    };
    auto pipeline = x | std::views::transform(get_path);

    std::ostream_iterator<std::string> dst{where, "\n included from "};
    std::ranges::copy(pipeline, dst);

    return where.str();
}

size_t Compiler::File::LineNum() const {
    return line_;
}

const std::filesystem::path& Compiler::File::Path() const {
    return path_;
}

}  // namespace karma