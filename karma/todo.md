### Features:

1. Check "address < memory_size" before accessing memory_

2. Check "register + 1 < registers_num" before accessing the next register
   * Write a comment on why a register cannot be out of bounds
     (it takes up 4 bits in command binary and we have 16 registers)

3. Allow to (optionally) define maximum stack size in constructor
   * default: unbounded (memory size)
   * if more than memory size -- make memory size
   * introduce "stack overflow" error
   * write a comment on why stack overflow cannot occur
     if stack size equals memory size
     - the stack would've rewritten the code segment

4. Write a comment on why memory size cannot be user-defined
   * the RM command format specs allows maximum 20 bits for memory address

5. Introduce strict compilation/execution mode that throws errors on user 
   access to r13-r15 registers or code/constants segments (+2 separate errors)

### Code refactor:

1. Rewrite all errors using `std::format` when GCC@13 is released

2. Concept definition: see `TODO` comments in [.clang-format](.clang-format)
   and [map.hpp](include/utils/map.hpp)