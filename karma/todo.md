### General:

1. Run include what you use on everything
2. Check all files for clang-tidy complaints (run clang-tidy directly, not through IDE)
3. Write a README in every folder (in the main one explain where are the tasks + 
   why so many C++ features are used) + list all namespaces
4. See if some functions need doxygen comments (especially in utils)
5. Write printf/scanf on Karma assembler
6. Review docs (latex + pdf), maybe add something

### Code refactor:

1. Rewrite all errors using `std::format` when GCC@13 is released

2. Concept definition: see `TODO` comments in [.clang-format](.clang-format)
   and [map.hpp](include/utils/map.hpp)
