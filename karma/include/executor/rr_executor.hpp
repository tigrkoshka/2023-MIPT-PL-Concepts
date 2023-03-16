#pragma once

#include <functional>     // for function
#include <unordered_map>  // for unordered_map

#include "executor/common_executor.hpp"
#include "executor/errors.hpp"
#include "executor/executor.hpp"
#include "specs/architecture.hpp"
#include "specs/commands.hpp"

namespace karma {

class Executor::RRExecutor : public CommonExecutor {
   private:
    using ExecutionError = errors::executor::ExecutionError::Builder;

    using Args      = detail::specs::cmd::args::RRArgs;
    using Operation = std::function<MaybeReturnCode(Args)>;

   private:
    detail::specs::arch::Word LHSWord(const Args&);
    detail::specs::arch::Double LHSDouble(const Args&);

    detail::specs::arch::Word RHSWord(const Args&);
    detail::specs::arch::Double RHSDouble(const Args&);

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
    using Map = std::unordered_map<detail::specs::cmd::Code, Operation>;
    Map GetMap();
};

}  // namespace karma