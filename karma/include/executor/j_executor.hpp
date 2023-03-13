#pragma once

#include <functional>     // for function
#include <unordered_map>  // for unordered_map

#include "common_executor.hpp"
#include "return_code.hpp"
#include "specs/commands.hpp"

namespace karma::executor::detail {

class JExecutor : public CommonExecutor {
   private:
    using Args      = karma::detail::specs::cmd::args::JArgs;
    using Operation = std::function<MaybeReturnCode(Args)>;

   private:
    Operation JMP();
    Operation JNE();
    Operation JEQ();
    Operation JLE();
    Operation JL();
    Operation JGE();
    Operation JG();
    Operation CALLI();
    Operation RET();

   public:
    using Map = std::unordered_map<karma::detail::specs::cmd::Code, Operation>;
    Map GetMap();
};

}  // namespace karma::executor::detail