### Features:

1. Put main.cpp and the outer CMakeLists.txt to a playground directory

### Code refactor:

1. Rewrite all errors using `std::format` when GCC@13 is released

2. Concept definition: see `TODO` comments in [.clang-format](.clang-format)
   and [map.hpp](include/utils/map.hpp)

### Executor:
#### Computer:

* Decompose memory and registers into separate class `Computer`:
  * `GetReg`, `PutReg`, `GetTwoReg`, `PutTwoReg`: check that a valid register is used
  * `GetMem`, `PutMem`, `GetTwoMem`, `PutTwoMem`: check that a valid memory address is used

* The public `Execute` methods are the `Computer`'s methods

#### Commands

* [maybe]: make a function for each command and store them to four maps:
  * `[RM | RI | RR | J]Commands`
  * 1 scenario: standalone functions additionally accepting a `Computer&`
    * [maybe]: use `std::bind` (`std::bind_front`, `std::bind_back`) to bind a `Computer&` in the `Executor` constructor
  * 2 scenario: methods of a class containing a `Computer&` as a member

* [maybe]: Decompose commands into a separate directory
  (including parsing and building)

#### Config:

Fields:

* `StackSize` (-1 for unbounded)
  * Default: `-1`
  * If more than memory size -- make `-1`
  * Stack overflow error

* Write a comment on why the memory size cannot be user-defined:
  it is closely related to the `RM` command format, in which 20
  bits are dedicated to the memory address

* `StrictRegisters` (to block access to `r13`-`r15`)
  * Default: `false`

* `StrictMemory` (to block access to code and constants segments)
  * Default: `false`

This config defaults to no restrictions.

This config is passed to the Computer's constructor
as well as to the Execute function, the strictest of these two
settings are used for a particular execution.

#### Minor:

* `registers_` -> `std::array<arch::Register, arch::kRegisters>`
* `syscall r0 0` returns the exit code from `r0`:
  * add to docs
  * fix the programs (in docs too) to clear the `r0` register before `syscall`
  * `Execute` returns the exit code