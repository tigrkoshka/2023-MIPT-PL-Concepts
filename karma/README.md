# Karma

This directory provides the implementation of
the [first task](https://clck.ru/33rsnH) of
the MIPT *Concepts of Programming Languages (2023)* course.

The compliance of this implementation to the original task is discussed
in the [Task.md](Task.md) file.

## Structure

This task implementation's code base turned out to be rather big, so there
are README files in each code directory to guide one through the main design
decisions and code parts interactions.

The list of subdirectories of this implementation and their main purpose is
given below in the preferred order of their exploration.

### Docs

The [docs](docs) directory contains a document [Karma.pdf](docs/Karma.pdf),
which describes the implemented Von Neumann architecture computer (Karma) and
an associated assembler language. It also contains the source files from which
the document was compiled.

### Include

The [include](include) directory contains the implementations of the *karma*
library, which mainly consists of a compiler, executor and disassembler for
the Karma assembler and executable files creation and usage.

For details on the implementation please refer to
the [README](include/README.md) in this directory.

### Programs

The [programs](programs) directory contains programs written in the Karma
assembler language.

For details on the contents of this directory please refer to its
[README](programs/README.md).

### Playground

The [playground](playground) directory contains a simple C++ program utilizing
the *karma* library.

For details on the usage of the code from this directory please refer to its
[README](programs/README.md).

## Building

### Automatic build

To automatically build of the *karma* library and the playground executable,
run the following command from this directory:

```shell
chmod +x build.sh && ./build.sh
```

### Results

After the command is executed, several new `build` directories will appear
containing the files produced via building a project with CMake.
Those directories can be ignored as they are only used internally by CMake.

Additionally, a `lib` directory will appear. It will contain a single file
`libkarma.a`, which is the archive of the compiled *karma* library.

In the [playground directory](playground) a `karma_play` executable file will
appear, which is the binary of the playground program.

### Compilers

The building was tested on Apple Clang 14.0.3 and GCC 12.2.0.

There is one specific feature of the *karma* library that only works when
compiling with GCC. That feature is the logging messages on multithreaded Karma
assembler program compilation. When compiled with Apple Clang those messages
may become interfered with one another, while a compilation with GCC prevents
that interference. That is due to the fact that the Clang version of STL has not
yet (as of March 2023) supported the C++20 `syncstream` library, which is used
to resolve the messages interference issue when compiling with GCC.

## C++ features used

The implementation heavily uses the C++20 standard features, sometimes even if
those are not absolutely necessary. That is because this task was taken not only
as an opportunity to better understand a computer architecture and assembler
languages, but also as a chance to learn more about modern C++ code design
and libraries.
