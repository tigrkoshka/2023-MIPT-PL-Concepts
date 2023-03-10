#pragma once

#include <concepts>  // for totally_ordered
#include <string>    // for string
#include <vector>    // for vector

#include "specs/architecture.hpp"
#include "specs/commands.hpp"
#include "specs/flags.hpp"

namespace karma::executor::detail {

class Impl {
   private:
    [[nodiscard]] karma::detail::specs::arch::TwoWords GetTwoRegisters(
        karma::detail::specs::cmd::args::Source) const;

    void PutTwoRegisters(karma::detail::specs::arch::TwoWords,
                         karma::detail::specs::cmd::args::Receiver);

    static void CheckBitwiseRHS(karma::detail::specs::arch::Word,
                                karma::detail::specs::cmd::Code);

    template <std::totally_ordered T>
    void WriteComparisonToFlags(T, T);

    void Jump(karma::detail::specs::flags::Flag,
              karma::detail::specs::cmd::args::Address);

    void Divide(karma::detail::specs::arch::TwoWords,
                karma::detail::specs::arch::TwoWords,
                karma::detail::specs::cmd::args::Receiver);

    bool Syscall(karma::detail::specs::cmd::args::Register,
                 karma::detail::specs::cmd::syscall::Code);

    void Push(karma::detail::specs::arch::Word);

    void Pop(karma::detail::specs::cmd::args::Receiver,
             karma::detail::specs::arch::Word);

    karma::detail::specs::cmd::args::Address Call(
        karma::detail::specs::cmd::args::Address);
    void Return();

    bool ExecuteRMCommand(karma::detail::specs::cmd::Code,
                          karma::detail::specs::cmd::args::Register,
                          karma::detail::specs::cmd::args::Address);

    bool ExecuteRRCommand(karma::detail::specs::cmd::Code,
                          karma::detail::specs::cmd::args::Receiver,
                          karma::detail::specs::cmd::args::Source,
                          karma::detail::specs::cmd::args::Modifier);

    bool ExecuteRICommand(karma::detail::specs::cmd::Code,
                          karma::detail::specs::cmd::args::Register,
                          karma::detail::specs::cmd::args::Immediate);

    bool ExecuteJCommand(karma::detail::specs::cmd::Code,
                         karma::detail::specs::cmd::args::Address);

    bool ExecuteCommand(karma::detail::specs::cmd::Bin);

    void ExecuteImpl(const std::string& exec_path);

   public:
    void MustExecute(const std::string& exec_path);
    void Execute(const std::string& exec_path);

    Impl()
        : memory_(karma::detail::specs::arch::kMemorySize),
          registers_(karma::detail::specs::arch::kNRegisters) {}

    ~Impl() = default;

    // Non-copyable
    Impl(const Impl&)            = delete;
    Impl& operator=(const Impl&) = delete;

    // Non-movable
    Impl(Impl&&)            = delete;
    Impl& operator=(Impl&&) = delete;

   private:
    std::vector<karma::detail::specs::arch::Word> memory_;

    std::vector<karma::detail::specs::arch::Register> registers_;

    karma::detail::specs::arch::Word flags_{0};
};

}  // namespace karma::executor::detail
