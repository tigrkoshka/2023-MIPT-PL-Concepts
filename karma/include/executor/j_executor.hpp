#pragma once

#include <functional>     // for function
#include <unordered_map>  // for unordered_map

#include "executor/common_executor.hpp"
#include "executor/executor.hpp"
#include "specs/commands.hpp"

namespace karma {

class Executor::JExecutor : public CommonExecutor {
   private:
    using Args      = detail::specs::cmd::args::JArgs;
    using Operation = std::function<MaybeReturnCode(Args)>;

   private:
    Operation JMP();
    Operation JNE();
    Operation JEQ();
    Operation JLE();
    Operation JL();
    Operation JGE();
    Operation JG();
    Operation PRC();
    Operation CALLI();
    Operation RET();

   public:
    using Map = std::unordered_map<detail::specs::cmd::Code, Operation>;
    Map GetMap();
};

}  // namespace karma
