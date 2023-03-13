### Code refactor:

1. Rewrite all errors using `std::format` when GCC@13 is released

2. Concept definition: see `TODO` comments in [.clang-format](.clang-format)
   and [map.hpp](include/utils/map.hpp)

### Executor:

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
