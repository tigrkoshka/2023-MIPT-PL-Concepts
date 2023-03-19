#include "constants.hpp"

#include <string>  // for string

#include "utils/map.hpp"

namespace karma::detail::specs::consts {

const std::unordered_map<Type, std::string> kTypeToName = {
    {UINT32, "uint32"},
    {UINT64, "uint64"},
    {DOUBLE, "double"},
    {CHAR,   "char"  },
    {STRING, "string"},
};

const std::unordered_map<std::string, Type> kNameToType =
    utils::map::Revert(kTypeToName);

}  // namespace karma::detail::specs::consts
