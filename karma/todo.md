### General:

1. Run include what you use on everything
2. Check all files for clang-tidy complaints (run clang-tidy directly, not through IDE)
3. Write a README in every folder (in the main one explain where are the tasks + 
   why so many C++ features are used)
4. See if some functions need doxygen comments (especially in utils)
5. Write printf/scanf on Karma assembler
6. Review docs (latex + pdf), maybe add something

### Code refactor:

1. Rewrite all errors using `std::format` when GCC@13 is released

2. Concept definition: see `TODO` comments in [.clang-format](.clang-format)
   and [map.hpp](include/utils/map.hpp)

### Executor:

#### Config:

Maybe rename executor to computer??

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
