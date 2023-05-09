# Exceptions

This directory provides the implementation of
the [second task](https://clck.ru/33rtCX) of
the MIPT *Concepts of Programming Languages (2023)* course.

The compliance of this implementation to the original task is discussed
in the [Task.md](Task.md) file.

## Structure

To guide one through this task's implementation, the main design decisions
and code parts interactions there are README files in each code directory.

The list of subdirectories of this implementation and their main purpose is
given below in the preferred order of their exploration.

### Include

The [include](include) directory contains the implementations of the *except*
library, the main parts of which are the automatic object lifetime manager
and the implementations of the exceptions throwing/catching algorithm.

For details on the implementation please refer to
the [README](include/README.md) in this directory.

### Playground

The [playground](playground) directory contains a simple C++ program utilizing
the *except* library.

For details on the usage of the code from this directory please refer to its
[README](programs/README.md).

## Building

To automatically build the *except* library and the playground executable,
run the following command from this directory:

```shell
chmod +x build.sh && ./build.sh
```

Both the *except* library and the playground are compiled with
the `-fno-exceptions` flag (supported by both Clang and GCC) to ensure
that the C++ native exceptions are not used.

To specify the compiler to be used (for the list of the tested compilers
see [below](#compilers)) one should set the environment variable `COMPILER`.
This can be done with the following command:

```bash
export COMPILER="<absolute_path_to_the_compiler>"
```

### Results

After the command is executed, several new `build` directories will appear
containing the files produced via building a project with CMake.
Those directories can be ignored as they are only used internally by CMake.

Additionally, a `lib` directory will appear. It will contain a single file
`libexcept.a`, which is the archive of the compiled *except* library.

In the [playground directory](playground) an `except_play` executable file will
appear, which is the binary of the playground program.

Additionally, if the Google Test framework is installed, the testing binaries
will appear in the directories containing the *except* library
[tests](include/tests) and the `AutoObject` class template
[tests](include/objects/tests). For more details on installing the Google Test
framework and running the tests please refer to those directories README files.

### Compilers

The building was tested on Clang 16.0.2 and GCC 13.1.0.

There are a couple workarounds for Clang and GCC internal bugs throughout
the except library code. These workarounds are marked with a `TODO` comment,
which describes the issue and the solution as well as provides a link to
the bugreport to track. Once the issues have been fixed by the compilers
developers, these workarounds will be eliminated.

## C++ features used

The implementation heavily uses the C++20 standard features, sometimes even if
those are not absolutely necessary. That is because this task was taken not only
as an opportunity to better understand a computer architecture and assembler
languages, but also as a chance to learn more about modern C++ code design
and libraries.
