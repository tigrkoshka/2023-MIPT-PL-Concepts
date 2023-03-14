#pragma once

#include <functional>     // for function
#include <unordered_map>  // for unordered_map

#include "executor/common_executor.hpp"
#include "executor/return_code.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"

namespace karma::executor::detail {

class RRExecutor : public CommonExecutor {
   private:
    using Args      = karma::detail::specs::cmd::args::RRArgs;
    using Operation = std::function<MaybeReturnCode(Args)>;

   private:
    karma::detail::specs::arch::Word LHSWord(const Args&);
    karma::detail::specs::arch::Double LHSDouble(const Args&);

    karma::detail::specs::arch::Word RHSWord(const Args&);
    karma::detail::specs::arch::Double RHSDouble(const Args&);

   private:
    Operation ADD();
    Operation SUB();
    Operation MUL();
    Operation DIV();
    Operation SHL();
    Operation SHR();
    Operation AND();
    Operation OR();
    Operation XOR();
    Operation ITOD();
    Operation DTOI();
    Operation ADDD();
    Operation SUBD();
    Operation MULD();
    Operation DIVD();
    Operation CMP();
    Operation CMPD();
    Operation MOV();
    Operation LOADR();
    Operation LOADR2();
    Operation STORER();
    Operation STORER2();
    Operation CALL();

   public:
    using Map = std::unordered_map<karma::detail::specs::cmd::Code, Operation>;
    Map GetMap();
};

}  // namespace karma::executor::detail