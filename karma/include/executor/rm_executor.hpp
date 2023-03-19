#pragma once

#include <functional>     // for function
#include <unordered_map>  // for unordered_map

#include "executor/common_executor.hpp"
#include "executor/executor.hpp"
#include "specs/commands.hpp"

namespace karma {

class Executor::RMExecutor : public CommonExecutor {
   private:
    using Args      = detail::specs::cmd::args::RMArgs;
    using Operation = std::function<MaybeReturnCode(Args)>;

   private:
    Operation LA();
    Operation LOAD();
    Operation LOAD2();
    Operation STORE();
    Operation STORE2();

   public:
    using Map = std::unordered_map<detail::specs::cmd::Code, Operation>;
    Map GetMap();
};

}  // namespace karma
