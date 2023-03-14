#pragma once

#include <memory>   // for shared_ptr
#include <string>   // for string
#include <utility>  // for move

#include "executor.hpp"
#include "executor/config.hpp"
#include "executor/j_executor.hpp"
#include "executor/return_code.hpp"
#include "executor/ri_executor.hpp"
#include "executor/rm_executor.hpp"
#include "executor/rr_executor.hpp"
#include "executor/storage.hpp"
#include "specs/commands.hpp"
#include "utils/traits.hpp"

namespace karma {

class Executor::Impl : detail::utils::traits::NonCopyableMovable {
   private:
    // use symbols from executor::detail namespace
    using ReturnCode      = executor::detail::ReturnCode;
    using MaybeReturnCode = executor::detail::MaybeReturnCode;
    using Storage         = executor::detail::Storage;
    using RMExecutor      = executor::detail::RMExecutor;
    using RIExecutor      = executor::detail::RIExecutor;
    using RRExecutor      = executor::detail::RRExecutor;
    using JExecutor       = executor::detail::JExecutor;

   private:
    MaybeReturnCode ExecuteCmd(karma::detail::specs::cmd::Bin);

   public:
    explicit Impl(Config config = Config())
        : storage_(std::make_shared<Storage>(std::move(config))) {}

    ReturnCode MustExecute(const std::string& exec, const Config& = Config());
    ReturnCode Execute(const std::string& exec, const Config& = Config());

   private:
    std::shared_ptr<Storage> storage_ = std::make_shared<Storage>();

    // we store the maps as const to avoid accessing them via the operator[]
    // and to force ourselves to check the .contains method before calling .at

    // NOLINTBEGIN(cppcoreguidelines-avoid-const-or-ref-data-members)
    RMExecutor rm_{storage_};
    const RMExecutor::Map rm_map_ = rm_.GetMap();

    RIExecutor ri_{storage_};
    const RIExecutor::Map ri_map_ = ri_.GetMap();

    RRExecutor rr_{storage_};
    const RRExecutor::Map rr_map_ = rr_.GetMap();

    JExecutor j_{storage_};
    const JExecutor::Map j_map_ = j_.GetMap();
    // NOLINTEND(cppcoreguidelines-avoid-const-or-ref-data-members)
};

}  // namespace karma
