### Features:

1. Multithreading in compilation
    * The `Labels` and the `Entrypoint` should be their own in
      the `FileCompiler`, with later merging, because the labels
      definitions are dependent on the size of the previously compiled
      code and constants.
    * Do not forget to update the main `compiler` directory
      [README](include/compiler/README.md) to specify this

2. Printing library: print `int64`

### General:

1. Write a README in every folder (in the main one explain where are the tasks +
   why so many C++ features are used) + list all namespaces
2. See if some functions need doxygen comments (especially in utils)
3. Write printf on Karma assembler
4. Review docs (latex + pdf), maybe add something

### Code refactor:

1. Rewrite all errors using `std::format` when GCC@13 is released

2. Concept definition: see `TODO` comments in [.clang-format](.clang-format)
   and [map.hpp](include/utils/map.hpp)
