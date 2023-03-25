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

### Exported

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

### Entrypoint

The `Entrypoint` class stores the information on the `end` directive of
the Karma assembler program.

A single instance of this class is created per Karma assembler file compilation.

#### Merge

Besides the getters and setters for the entrypoint address and the `end`
directive position in the program, this class provides an additional `Merge`
method, which allows to merge two instances of the `Entrypoint` class
into a single one.

The `Merge` method simply selects the entrypoint from whichever instance
specifies it. If both instances specify an entrypoint, a compilation error
is thrown, which is in accordance to the 'one entrypoint per program' rule
of the Karma assembler standard.

### Labels

The `Labels` class is used to manage the labels defined in the Karma assembler
program.

A single instance of this class is created per Karma assembler file compilation.

#### CheckLabel

Besides the getters and setters for the labels definitions and usages,
this class provides a static `CheckLabel` method that checks the validity of
a label name and throws a `CompileError` exception in case it is invalid.

#### Merge

Additionally, the `Labels` class provides a `Merge` method, which allows
to merge two instances of this class into a single one.

The `Merge` method accepts an rvalue of `Lables` type and the shifts that need
to be applied to the code and constants labels definitions as well as to
the labels usages (see [below](#data) for details).

#### SetCodeSize

The `Labels` class also provides a `SetCodeSize` method, after calling which
the constants labels definitions obtained via the `TryGetDefinition` will
be shifted by the value passed to the `SetCodeSize` method. This is used on
the labels substitution phase of compilation (see [below](#toexecdata)
for details).

### Data

The `Data` class represents the intermediate result of Karma assembler
compilation (i.e. the code and the constants segments, as well as
the entrypoint and the labels defined in the file).

An instance of this class is created once per Karma assembler file compilation
as its result. The data is filled directly by the methods of the only friend
class `FileCompiler` (see [below](#file-compiler) for details). This means that
the files' compilations are independent.

The class provides two public methods: `MergeAll` and `ToExecData`.

#### MergeAll

The `MergeAll` static method merges several instances of the `Data` class
resulting from each Karma assembler file compilation into a single `Data` class
instance representing the whole program.

*Code and constants*

The code and the constants segments are simply appended to one another.

*Entrypoint*

The resulting instance's entrypoint is the one from the original instances
which is not empty.

> **Note**
>
> Such an entrypoint must be exactly one since a Karma assembler program must
> have exactly one entrypoint. If there are several non-empty entrypoints in
> the original `Data` instances, that means that several files of the program
> have an `end` directive, which causes a compilation error.

*Labels*

The labels are combined so that all definitions and usages are preserved, but
shifted if necessary.

That is, when a Karma assembler file is being compiled, the recorded labels'
definitions (i.e. the addresses of the commands or constants represented by
the labels) and the addresses of the commands that use them are relative
to the start of the code/constants segment of the file.
When combining the labels from all files, the command labels definitions and
the labels usages positions are shifted (i.e. increased) by the size of
the combined code segment of the previous files (in the order of inclusion,
see [below](#includesmanager)). Similarly, the constants labels definitions
are shifted by the size of the combine constants segment of the previous files.

> **Note**
>
> For the constants labels this shift still does not produce the absolute
> addresses of the referenced constants. Instead, it produces the addresses
> relative to the beginning of the constants segment of the resulting Karma
> executable file. The shift by the total code segment size needed to obtain
> the absolute addresses of the constants is done on the labels substitution
> phase (see [below](#toexecdata) for details).
>
> To get the absolute addresses of the constants, an additional shift by
> the total code segment size is needed. If we were to apply this shift here,
> we would have needed to do that on each `MergeAll` call. However, we
> only need those absolute addresses when passing them as parameters to
> the compiled instructions, i.e. on the labels substitution phase.

Potentially the following `MergeAll` usage is allowed:

* Suppose we have two groups of Karma assembler files, group `A` and group `B`

* Let's assume all the files of all groups have been compiled independently,
  producing their own instances of the `Data` class

* Now one may call the `MergeAll` method to combine the `Data` class instances
  of the files from the group `A` into a single `Data` class instance
  representing
  the whole group. The same can be done for the group `B`

* Now one may call the `MergeAll` method again, passing it the resulting `Data`
  class instances for both groups to obtain a new `Data` class instance
  representing both groups of files combined

Currently, the described possibility is not used by the `Karma` compiler since
it does not currently support compiling parts of a `Karma` program independently
and can only compile an entire program into a single `Karma` executable file.

> **Note**
>
> This possibility imitates the ability of the real-world compilers to compile
> parts of code independently, store the results as archives, and then link
> the resulting archives to one another.

#### ToExecData

The `ToExecData` consumes a `Data` class instance (by only being available
for rvalues of the `Data` class type) and produces the data to be written
the Karma executable file (see the exec directory [README](../exec/README.md)
for details).

This method:

* Checks that an entrypoint is specified by the `Data` class instance it
  is called on (if not, a compilation error is thrown)

* Performs the labels substitution after applying the shift by the code segment
  size to the constants labels definitions

* Combines all the data into an `Exec::Data` class instance

> **Note**
>
> The shift of the constants labels definitions is performed *virtually*, i.e.
> the actual definitions are not traversed. Instead, a special `SetCodeSize`
> method is called on the `Labels` class instance to get the shifted definitions
> during labels substitution (see [above](#labels) for details).

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
an `std::vector` of the `File` class instances (inside `std::unique_ptr`s).
It analyses all the `include` directives in the original file and the files
included in it (recursively) to produce `File` class instances for
the original file as well as for every included file (transitively).

The order in which the files are stored inside the resulting `std::vector`
is the same as the order in which the contents of said files would be written
into the original file if the includes were to be 'substituted'.
If a file is included several times, only the first inclusion
is recorded.

That is, it performs a post-order traversal of the Karma assembler files
inclusion tree with a single-entering policy. For the single-entering policy
implementation, each file is assigned an identification value equal to its
absolute path in the filesystem.

This method guarantees that no more than one file is simultaneously opened.

`GetFiles` throws a `CompileError` if any `include` directives in any file
have an invalid syntax.

For details on the file inclusion rules and syntax see
the *Include directive* section of the [docs](../../docs/Karma.pdf).

### File compiler

The `FileCompiler` class methods perform the most part of the compilation
business logics.

A single `FileCompiler` class instance is created per Karma assembler file.
The constructor of the `FileCompiler` class accepts a `File` class instance
specifying the file to be compiled.

When created, a `FileCompiler` class instance can only be used once as
an rvalue since it provides a single rvalue method `PrepareData`, which
compiles the Karma assembler file specified in the constructor and returns
the produced code and constants segments as well as the records of an entrypoint
and labels (if those are specified in the file) inside an instance of
the `Data` class.

If the address operand was provided to an `RM` or `J` format command via
a label, the `PrepareData` method of the `FileCompiler` class leaves
the operand's bits blank in the respective commands' binaries. Those blanks
are filled on the labels substitution phase of the compilation process
(see [above](#data) for details).

### Impl

The `Impl` class is a static class, whose methods combine the functionality
of the classes discussed above and perform the full Karma assembler
program compilation.

The compilation stages are:

* Getting the list of files to be compiled using the functionality of
  the `IncludesManager` class (see [above](#includesmanager) for details)

* Compiling all files separately using the functionality of
  the `FileCompiler` class (see [above](#file-compiler) for details)

* Combining the `Data` class instances produced by the `FileCompiler`s
  into a single `Data` class instance using the `MergeAll` static method of
  the `Data` class (see [above](#mergeall) for details)

* Substituting labels (i.e. filling the blanks in the commands' binaries
  left by the `PrepareData` method of the `FileCompiler` class) and producing
  the Karma executable file contents using the `ToExecData` rvalue method of
  the `Data` class (see [above](#toexecdata) for details)

* Writing the data to the resulting Karma executable file via the `Exec::Write`
  method (see the exec directory [README](../exec/README.md) for details).

### Compiler

The `Compiler` class is an exported static class wrapping the methods of
the `Impl` class. It is also used as the namespace for all the classes
described above.

The exported methods of the `Compiler` class accept a single compulsory
parameter specifying the path to the main Karma assembler file to be compiled
as well as the following optional parameters:

* **Destination**: the path of the resulting Karma executable file, defaults
  to a file in the same directory as the provided main Karma assembler file,
  with the same name, and with the last extension replaced with `.a`

* **Number of workers**: the number of concurrent workers (threads) used to
  compile the Karma assembler program, defaults to the implementation-defined
  value returned from `std::thread::hardware_concurrency()` or 1, if that value
  is 0. The number of used workers is always not greater than the number of
  Karma assembler files in the program (i.e. file compilation is an atomic
  routine), if it is specified to be greater than the number of files, it is
  reduced to it, and each thread compiles a single file

* **Logger**: the output stream to print the compilation process info, defaults
  to a no-op stream (i.e. the one that drops the messages instead of printing
  them)

> **Note**
>
> The overloads of the public methods of the `Compiler` class are designed so
> that the unused optional parameters may be omitted as long as the used ones
> come in the same relative order as listed above, i.e. all the following calls
> are valid:
>
> ```c++
> karma::Compiler::Compile(/* src = */ "main.krm",
>                          /* dst = */ "main.out",
>                          /* n_workers = */ 4,
>                          /* log = */ std::clog);
> 
> karma::Compiler::Compile(/* src = */ "main.krm",
>                          /* dst = */ "main.out",
>                          /* n_workers = */ 4);
> 
> karma::Compiler::Compile(/* src = */ "main.krm",
>                          /* dst = */ "main.out",
>                          /* log = */ std::clog);
> 
> karma::Compiler::Compile(/* src = */ "main.krm",
>                          /* dst = */ "main.out");
> 
> karma::Compiler::Compile(/* src = */ "main.krm",
>                          /* n_workers = */ 4,
>                          /* log = */ std::clog);
> 
> karma::Compiler::Compile(/* src = */ "main.krm",
>                          /* n_workers = */ 4);
> 
> karma::Compiler::Compile(/* src = */ "main.krm",
>                          /* log = */ std::clog);
> 
> karma::Compiler::Compile(/* src = */ "main.krm");
> ```
