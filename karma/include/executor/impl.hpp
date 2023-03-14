#pragma once

#include <memory>   // for shared_ptr
#include <string>   // for string
#include <utility>  // for move

#include "config.hpp"
#include "j_executor.hpp"
#include "return_code.hpp"
#include "ri_executor.hpp"
#include "rm_executor.hpp"
#include "rr_executor.hpp"
#include "specs/commands.hpp"
#include "storage.hpp"
#include "utils/traits.hpp"

namespace karma::executor::detail {

class Impl : karma::detail::utils::traits::NonCopyableMovable {
   private:
    MaybeReturnCode ExecuteCommand(karma::detail::specs::cmd::Bin);

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

}  // namespace karma::executor::detail
