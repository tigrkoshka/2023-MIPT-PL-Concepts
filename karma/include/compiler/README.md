# Compiler

## Overview

This directory provides the compiling part of the public interface
of the [karma library](..).

## Dependencies

Both the declarations and the definition of the symbols from
this directory are dependent on and only on the symbols provided by
the [utils directory](../utils), the [specs directory](../specs)
and the [exec directory](../exec).

## Symbols

### Public

```c++
karma::                                 // compiler.hpp
        Compiler::
        |       MustCompile
        |       Compile
        |
        errors::
                compiler::
                        Error
                        InternalError
                        CompileError
```

### Internal

```c++
karma::
        Compiler::
        |       Entrypoint              // entrypoint.hpp
        |       Labels                  // labels.hpp
        |       Data                    // data.hpp
        |       File                    // file.hpp
        |       IncludesManager         // includes.hpp
        |       FileCompiler            // file_compiler.hpp
        |       Impl                    // impl.hpp
        |               
        errors::                        // errors.hpp
                compiler::
                        InternalError::Builder
                        CompileError::Builder
```

## Design description

### Entrypoint and Labels

The `Entrypoint` and the `Labels` classes are used to manage
the `end` directive and the labels in the Karma assembler program respectively.

These are the concepts that logically connect the program as a whole in
the sense that there must be exactly one entrypoint in the program and
the labels must be unique throughout the whole program.

These classes are used as 'dummy' storage, i.e. they simply provide methods
to record the occurrences of the `end` directive and the labels in the program.
The `Labels` class additionally provides a static `CheckLabel` method that
checks the validity of a label name and throws a `CompileError` exception
in case it is invalid.

The instances of these classes are created once per Karma assembler file
compilation.

Both these classes provide an additional `Merge` method, which allows to merge
the instances of these classes created for each Karma assembler file into
a single instance representing the while program
(see [below](#data) for details).

### Data

The `Data` class represents the data from a single Karma assembler file (i.e.
the code, the constants, the entrypoint and the labels defined in the file).

It is a class without any business logics used only to 'centralize' access
to the code and the constants of a single file.

An instance of this class is created once per Karma assembler file compilation.

### File

The `File` class provides the tools for reading a Karma assembler file.
It additionally keeps track of its own absolute path as well as a pointer to
the `File` instance corresponding to the Karma assembler file from which
the Karma assembler file represented by the original `File` instance
was included, which allows to produce more informative `CompileError` messages
identifying the exact line, file, and the full `include` path to the file where
the error occurred.

A single instance of this class is created for each Karma assembler file.
The instances are stored in `std::unique_ptr`s and are reused in several
compilation stages.

### IncludesManager

The `IncludesManager` class provides a single `GetFiles` method that can only be
called on an rvalue of `IncludesManager` type. That means that any instance
can only be used once.

An `IncludesManager` class instance is created once per Karma assembler
program compilation.

The `GetFiles` method accepts the path to a Karma assembler file and produces
an `std::vector` of `File` class instances (inside `std::unique_ptr`s).
It analyses all the `include` directives in the original file and the files
included in it (recursively) to produce `File` class instances for
the original file as well as for every included file (transitively).

The order in which the files are stored inside the resulting `std::vector`
is the same as the order in which the contents of said files would be written
into the original file if the includes were to be 'substituted'.
If a file is included several times, only the first inclusion
is recorded.
That is, it performs a post-order traversal of the Karma assembler files
inclusion tree with a single-entering policy.
To implement the single-entering policy, each file is assigned an identification
value equal to its absolute path in the filesystem.

This method guarantees that no more than one file is simultaneously opened.

`GetFiles` throws a `CompileError` if any `include` directive in any file
has an invalid syntax.

For details on the file inclusion rules and syntax see
the *Includes* section of the [docs](../../docs/Karma.pdf).

### File compiler

The `FileCompiler` class methods perform the most part of the compilation
business logics.

The constructor of the `FileCompiler` class accepts a `File` class instance
specifying the file to be compiled, `std::shared_ptr`s to the `Labels` and
the `Entrypoint` class instances to record the respective processed file
information in them, and the sizes of the previously compiles code and
constants segments to be able to calculate the correct labels values.

A single `FileCompiler` class instance is created per Karma assembler file.

When created, a `FileCompiler` class instance can only be used once as
an rvalue since it provides a single rvalue method `PrepareData`, which
compiles the Karma assembler file specified in the constructor and returns
the produced parts of the resulting code and constants segments inside
an instance of the `ExecData` class.

The `PrepareData` method of this class leaves the bits of the commands' binaries
which specify the address operand blank in case the address operand was provided
via a label.

### Impl

The `Impl` class is a static class, whose methods combine the functionality
of the classes discussed above and performs the full Karma assembler
program compilation.

The compilation stages are:

* Getting the list of files to be compiled using the functionality of
  the `IncludesManager` class

* Compiling all files separately using the functionality of
  the `FileCompiler` class

* Labels substitution, i.e. filling the blanks in the commands' binaries
  left by the `PrepareData` method of the `FileCompiler` class

* Combining the `Data` class instances produced by the `FileCompiler`s
  as well as the `Entrypoint` class data prepared on the previous stage
  into a single `Exec::Data` struct instance (see the exec directory
  [README](../exec/README.md) for details).

* Writing the data to the resulting Karma executable file via the `Exec::Write`
  method (see the exec directory [README](../exec/README.md) for details).

### Compiler

The `Compiler` class is an exported static class simply wrapping the methods
of the `Impl` class. It is also used as the namespace for all the classes
described above.
