#pragma once

#include <string>  // for string
#include <vector>  // for vector

#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "utils/traits.hpp"

namespace karma::detail::exec {

struct Data : karma::detail::utils::traits::NonCopyableMovable {
    specs::arch::Address entrypoint;
    specs::arch::Address initial_stack;

    std::vector<specs::cmd::Bin> code;
    std::vector<specs::arch::Word> constants;
};

void Write(const Data& data, const std::string& exec_path);
Data Read(const std::string& exec_path);

}  // namespace karma::detail::exec