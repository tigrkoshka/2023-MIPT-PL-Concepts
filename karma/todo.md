### Features:

1. Check "address < memory_size" before accessing memory_

2. Check "register + 1 < registers_num" before accessing the next register

3. Introduce constants (uint, uint64, double, string)
   * think on exec format:
     - type identifier before each constant for the disassembler -- ?
     - compilation flag --optimize-exec that disallows disassembling -- ?

4. Allow to (optionally) define maximum stack size in constructor
   * default: unbounded (memory size)
   * if more than memory size -- make memory size
   * introduce "stack overflow" error
   * write a comment on why stack overflow cannot occur
     if stack size equals memory size
     - the stack would've rewritten the code segment

5. Write a comment on why memory size cannot be user-defined
   * the RM command format specs allows maximum 20 bits for memory address

6. Introduce strict compilation/execution mode that throws errors on user 
   access to r13-r15 registers or code/constants segments (+2 separate errors)

### Code refactor:

1. Split `karma::details::specs` namespace:
   * `karma::details::specs::cmd`
   * `karma::details::types`

2. Maybe rethink errors? Options:
   * namespaces
     - cons: global access + too long identifiers or more "using namespace"
   * Declaration to .cpp
     - cons: they do not hurt the .hpp

3. Rewrite all errors using `std::format` when GCC@13 is released

4. Concept definition: see `TODO` comments in [.clang-format](.clang-format)
   and [utils.hpp](src/utils/utils.hpp)