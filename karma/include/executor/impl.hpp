#pragma once

#include <memory>  // for shared_ptr
#include <string>  // for string

#include "j_executor.hpp"
#include "return_code.hpp"
#include "ri_executor.hpp"
#include "rm_executor.hpp"
#include "rr_executor.hpp"

namespace karma::executor::detail {

class Impl : karma::detail::utils::traits::NonCopyableMovable {
   private:
    MaybeReturnCode ExecuteCommand(karma::detail::specs::cmd::Bin);

   public:
    Impl()
        : storage_(std::make_shared<Storage>()) {}

    ReturnCode MustExecute(const std::string& exec_path);
    ReturnCode Execute(const std::string& exec_path);

   private:
    std::shared_ptr<Storage> storage_;

    RMExecutor rm_{storage_};
    const RMExecutor::Map rm_map_ = rm_.GetMap();

    RIExecutor ri_{storage_};
    const RIExecutor::Map ri_map_ = ri_.GetMap();

    RRExecutor rr_{storage_};
    const RRExecutor::Map rr_map_ = rr_.GetMap();

    JExecutor j_{storage_};
    const JExecutor::Map j_map_ = j_.GetMap();
};

}  // namespace karma::executor::detail
