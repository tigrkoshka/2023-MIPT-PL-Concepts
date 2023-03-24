# Karma library

## Overview

The `karma` library provides tools to compile Karma assembler programs and
execute the resulting Karma executable file as well as to disassemble
an existing Karma executable file back into the Karma assembler language.

Thus, the karma library consists of three _blocks_: `compiler`, `executor` and
`disassembler`. Each block consists of a class in the `karma` namespace
and three types of errors in the `karma::errors::<block_name>` namespace.

## Exported symbols

```c++
karma::
        Compiler::                       // compiler block
        |       MustCompile
        |       Compile
        |
        Executor::                       // executor block
        |       MustExecute
        |       Execute
        |       Config::
        |               /* 
        |                * various methods for
        |                * configuration
        |                * setup and lookup
        |                */
        |       
        Disassembler::                   // disassembler block
        |       MustDisassemble
        |       Disassemble
        |       
        errors::
                Error
                |
                compiler::               // compiler block
                |       Error
                |       InternalError
                |       CompileError
                |       
                executor::               // executor block
                |       Error
                |       InternalError
                |       ExecutionError
                |       
                disassembler::           // disassembler block
                        Error
                        InternalError
                        DisassembleError
```

## Classes

### Compiler and Disassembler

The `karma::Compiler` and the `karma::Disassembler` classes provide only
static methods for compiling Karma assembler programs and decompiling
Karma executable files respectively.

### Executor

The `karma::Executor` class emulates the Karma computer by allocating
the address space and the registers in its constructor and providing methods
to execute Karma executable files. Multiple files may be executed using
one instance of `karma::Executor` (much like on a computer).

#### Config

Both the `karma::Executor` constructor and the methods for execution
accept an optional `karma::Executor::Config` argument, which sets
the configuration for the emulated Karma computer and a specific execution
respectively. See the *Execution configuration* section of the
[docs](../docs/Karma.pdf) for details on the configuration semantics.

The `karma::Executor::Config` class has various methods for the configuration
setup and lookup as well as presets of commonly used configurations.
See the executor block [README](executor/README.md) for details.

## Methods

### Variants

All exported methods of the karma library classes have two variants,
one of which has a `Must` prefix, and the other does not. The former throw
exceptions (see [below](#exceptions)), while the latter wrap them in
a `try-catch` block and print the exception info to `stderr`.

### Overloads

All these methods accept a single compulsory parameter and several optional
parameters. The overloads of these methods are designed so that any optional
parameters may be skipped and any other may be specified at the same time.
That is achieved by making all the optional parameters have different types.

The last optional parameter of all the methods is the logger, which allows
to specify the output stream for logging the compilation, execution and
disassembling. The default value of this parameter is a no-op logger,
i.e. a logger that drops the messages and does not print them anywhere.

For the list of all supported optional parameters and their default values
please refer to the `README`s of the respective directories:
[compiler](compiler/README.md), [executor](executor/README.md) and
[disassembler](disassembler/README.md).

### Return values

The methods of the `karma::Compiler` and the `karma::Disassembler` classes
do not return anything but rather output the results of
the compilation/disassembling to the specified output.

The methods of the `karma::Executor` class return the exit code of the executed
program.

## Exceptions

It is guaranteed that any exception thrown by the public methods of the karma
library is either of type `karma::errors::Error` or of a type derived
from it. The type `karma::errors::Error` itself is derived
from `std::runtime_error`.

Each block defines three new types of exceptions:

* `karma::<block_name>::Error` is the base type for two other
  exceptions types of each block and is not usually thrown directly.
  An exception of *precisely* this type indicates an unexpected bug
  in the `karma` library and should be reported as an issue.

* `karma::<block_name>::InternalError` indicates a bug in the `karma` library
  that was caught by internal checks, and usually provides informative debug
  info. If thrown, should be reported as an issue with the debug info
  attachment.

* The third exception type indicates an issue with the Karma assembler program
  or Karma executable file contents provided by the user. These are the only
  exception types directly related to the semantics of the methods.

It is guaranteed that any exception thrown by the methods in a `karma`
library block is either of type `karma::errors::<block_name>::Error` or of
a type derived from it.

Therefore, an exception-safe usage of the `karma` library method with
the `Must` prefix looks somewhat like this:

```c++
try {
    karma::Compiler::MustCompile(karma_assembler_file_path);
} catch (const karma::errors::compiler::CompileError& e) {
    // handle a compile-time error of the provided assembler code
} catch (const karma::errors::compiler::InternalError& e) {
    // handle an internal compiler error
} catch (const karma::errors::compiler::Error& e) {
    // handle an error thrown by the compiler,
    // but not directly related to the compilation process
}
```

or simply:

```c++
try {
    karma::Compiler::MustCompile(karma_assembler_file_path);
} catch (const karma::errors::compiler::Error& e) {
    // handle any possible error
}
```

An exception-safe `karma` library usage either only uses exported methods
without the `Must` prefix, or looks somewhat like this:

```c++
try {
    karma::Compiler::MustCompile(karma_assembler_file_path,
                                 karma_executable_file_path);
    
    karma::Executor executor;
    auto _ /* return_code*/ = executor.MustExecute(karma_executable_file_path);
    
    karma::Disassembler::MustDisassemble(karma_executable_file_path,
                                         karma_disassembled_file_path)
    
} catch (const karma::errors::compiler::CompileError& e) {
    // handle a compile-time error of the provided assembler code
} catch (const karma::errors::compiler::InternalError& e) {
    // handle an internal compiler error
} catch (const karma::errors::compiler::Error& e) {
    // handle an error thrown by the compiler,
    // but not directly related to the compilation process
} catch (const karma::errors::executor::ExecutionError& e) {
    // handle a runtime error of the provided Karma assembler program
} catch (const karma::errors::executor::InternalError& e) {
    // handle an internal executor error
} catch (const karma::errors::executor::Error& e) {
    // handle an error thrown by the executor,
    // but not directly related to the execution process
} catch (const karma::errors::disassembler::DisassembleError& e) {
    // handle an error that occurred during the Karma executable disassembling
    
    // NOTE: in this particular case such an error should not occur
    //       since the executable file was generated by the karma compiler,
    //       this catch clause is here just as an example
} catch (const karma::errors::disassembler::InternalError& e) {
    // handle an internal disassembler error
} catch (const karma::errors::disassembler::Error& e) {
    // handle an error thrown by the disassembler,
    // but not directly related to the disassembling process
}
```

or simply:

```c++
try {
    karma::Compiler::MustCompile(karma_assembler_file_path,
                                 karma_executable_file_path);
    
    karma::Executor executor;
    auto _ /* return_code*/ = executor.MustExecute(karma_executable_file_path);
    
    karma::Disassembler::MustDisassemble(karma_executable_file_path,
                                         karma_disassembled_file_path)
    
} catch (const karma::errors::Error& e) {
    // handle any possible error
}
```
