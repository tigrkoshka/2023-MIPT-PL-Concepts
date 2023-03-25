# Executor

## Overview

This directory provides the execution part of the public interface
of the [*karma* library](..).

## Dependencies

The declarations of the symbols from this directory are dependent on and only on
the symbols provided by the [utils directory](../utils)
and the [specs directory](../specs).

The definitions are additionally dependent on the symbols provided
by the [exec directory](../exec).

## Symbols

### Exported

```c++
karma::
        Executor::                      // executor.hpp
        |       MustExecute
        |       Execute
        |       Config::                // config.hpp
        |               /* 
        |                * various methods for
        |                * configuration
        |                * setup and lookup
        |                */
        |
        errors::                        // executor.hpp
                executor::
                        Error
                        InternalError
                        ExecutionError
```

### Internal

```c++
karma::
        Executor::
        |       Storage                 // storage.hpp
        |       ExecutorBase            // executor_base.hpp
        |       CommonExecutor          // common_executor.hpp
        |       RMExecutor              // rm_executor.hpp
        |       RRExecutor              // rr_executor.hpp
        |       RIExecutor              // ri_executor.hpp
        |       JExecutor               // j_executor.hpp
        |       Impl                    // impl.hpp
        |       Config::
        |               AccessConfig    // config.hpp
        |               
        errors::                        // errors.hpp
                executor::
                        InternalError::Builder
                        ExecutionError::Builder
```

## Design description

### Storage

The `Storage` class represents and encapsulates the address space and
the registers of a Karma computer.

It is used as a centralized access point to both memory and registers, which
allows for consistent access managing, including access limiting specified by
the Karma computer and program execution configurations.

A `Config` class instance is accepted by both the `Storage` class constructor
and the `PrepareForExecution` method. The former specifies the Karma computer
configuration, and the latter affects a single Karma program execution.
For a specific execution, the strictest combination of these configurations
is used (see [below](#config) for details).

A single `Storage` class instance is created per an `Executor` class instance,
and then distributed via `std::shared_ptr`s to all the other classes that need
to interact with the storage.

### ExecutorBase

The `ExecutorBase` class wraps a `Storage` class instance
(in an `std::shared_ptr`) and 'forwards' all the methods of the `Storage` class
(i.e. provides methods with the same signature simply calling the respective
methods of the `Storage` class).

This class is used as the base class for all other classes implementing
the Karma computer business logic. It allows for a single `Storage` class
instance to be created per an `Executor` class instance and then shared between
all the classes. This could not have been achieved were the other classes
to be derived directly from the `Storage` class, so such a wrapper and method
forwarder is needed.

No `ExecutorBase` class instances are created directly, they are only created as
parent instances of [CommonExecutor](#commonexecutor) class instances.

### CommonExecutor

The `CommonExecutor` class is derived from `ExecutorBase` and provides reused
helper methods as well as methods managing the `flags` register, the stack,
and the function calls. These methods are then used in the
[`[RM|RR|RI|J]Executor`](#rm--rr--ri--jexecutor) classes business logics.

No `CommonExecutor` class instances are created directly, they are only created
as parent instances of [`[RM|RR|RI|J]Executor`](#rm--rr--ri--jexecutor)
classes instances.

### \[RM | RR | RI | J\]Executor

These classes are derived from the `CommonExecutor` class and provide the main
part of the Karma assembler commands business logic.

Each of these classes represents the commands of the respective format.
The public interface of each of these classes provides a single `GetMap` method,
which returns a mapping from the command code to an `std::function` implementing
the command's business logic.

These functions are created as lambda expressions inside the private methods
of these classes (a private method per Karma assembler command) and capture
the `this` pointer of the respective class, which provides access
to the Karma computer address space and registers via the methods of
the `ExecutorBase` class from which these classes are indirectly (through
the `CommonExecutor` class) derived.

Because of the `this` pointer capturing in the lambda expressions, the instances
of these classes must be preserved while the functions provided by the mapping
returned from the `GetMap` method are used.

A single instance of each of these classes is created per an `Executor` class
instance, and the `GetMap` method of each of them is only used once.

> **Note**
>
> One of the different designs considered for Karma commands implementation was
> to make a `switch` statement for each command format, and implement
> the commands' business logics inside the `case`s of those switch
> statements.
>
> While `switch` statements are more efficient than calling a stored
> `std::function`, this approach was dismissed, because it did not allow for
> much decomposition and involved huge `switch` statements, which tended
> to be bugprone, and which would only grow were new commands to be introduced
> in the Karma assembler.

> **Note**
>
> An alternative implementation of this approach could have avoided having to
> store instances of these classes, and instead making all the methods of these
> classes static, accepting an `std::shared_prt` to the `Storage` class
> instance, capturing this `std::shared_ptr` in the lambda expressions and
> creating an instance of the `CommonExecutor` class inside each lambda.
>
> This design was dismissed because of its two drawbacks.
>
> The first one is that the `std::shared_ptr` needs to be copied for each
> command implementing method of these classes, and were new commands
> to be introduces to the Karma assembler, the number of such copies would grow.
>
> The second one is that the need for each method to accept an `std::shared_ptr`
> and for each lambda expression to create its own instance of
> the `CommonExecutor` class made the code much less concise and its
> development inconvenient.

### Impl

The `Impl` class implements the main part of the Karma computer business logic.

Its constructor is the place where the instance of the `Storage` class is
created. It also creates the instances of
the [`[RM|RR|RI|J]Executor`](#rm--rr--ri--jexecutor)
classes and stores the mappings returned by their `GetMap` methods
(see [above](#rm--rr--ri--jexecutor) for details).

An instance of the `Impl` class is created once per an `Executor` class
instance.

The public methods of this class accept the path to the Karma executable file,
read the data from it via the `Exec::Read` method (see the exec directory
[README](../exec/README.md) for details), emulate the execution of the file
on a Karma computer, and return the return code of the execution after it is
finished.

These methods parse each command for its code and arguments and execute those
commands with the methods specified by the stored mappings returned from
the `GetMap` methods of the [`[RM|RR|RI|J]Executor`](#rm--rr--ri--jexecutor)
classes. They also manage the instruction pointer register to proceed to the
next executed instruction (see the *r15 register* paragraph of
the *Karma calling convention* section of the [docs](../../docs/Karma.pdf) for
details).

> **Note**
>
> The instruction pointer is advanced before a command is executed, which is in
> accordance to the rule that during a command execution the instruction
> register contains the address of the *next* executed instruction, which is
> essential for the function call procedure (see the *Karma calling convention*
> section of the [docs](../../docs/Karma.pdf) for details).

All the public methods of this class accept an additional optional parameter of
the `Config` class type, which specifies the configuration of the specific
execution (see [below](#config) for details).

### Config

The `Config` class represents the configuration of a Karma computer or a single
Karma program execution.

It provides various methods for updating its fields as well as for extracting
their contents. For the full list of the configuration options refer to
the *Execution configuration* section of the [docs](../../docs/Karma.pdf).

It also provides overloaded `operator&=` and `operator&`, which combine two
`Config` instances setting the *strictest* combination of their options
(for the definition of the strictest combination of two configs refer to
the *Execution configuration* section of the [docs](../../docs/Karma.pdf)).

Each exported constructor/method that accepts an instance of the `Config` class,
accepts it as an optional parameter. The default value for that parameter is
always the configuration not setting any blocks on either registers of address
space and not bounding the stack size, which is in accordance with the
[docs](../../docs/Karma.pdf).

#### Presets

The `Config` class additionally provides convenient presets not specified
in the [docs](../../docs/Karma.pdf). The presets are static functions returning
an instance of the `Config` class with specific predefined values of its fields.

Currently, there are two presets: `ExtraStrict` and `Strict`.

*ExtraStrict*

The `ExtraStrict` preset blocks the read and write access to the stack pointer,
stack frame pointer and instruction pointer registers (i.e. `r13`–`r15`)
as well as to the code and constants segments of the code.

> **Warning**
>
> If the `ExtraStrict` preset is used for an execution, the execution cannot
> access the constants defined in the program (because of the read block on
> the constants segment), and no functions using the Karma calling convention
> can obtain the values of their arguments (because of the read block on
> the stack pointer register). Therefore, this preset should be used
> with caution.

*Strict*

The `Strict` preset is like `ExtraStrict`, but lifts the read block on
the stack pointer register and the constants segment, which allows to avoid
the inconveniences of the `ExtraStrict` preset mentioned in the note above.

The `Config` constructed by the `Strict` preset is the one recommended
to be passed to the `Executor` class constructor.

> **Note**
>
> Though recommended, the configuration produced by this preset is not
> the default one, because the [docs](../../docs/Karma.pdf) specify that
> the default configuration should indicate no blocks at all.

#### AccessConfig

The `AccessConfig` class is used to decompose the blocks set to some registers
and/or Karma computer address space segments.

The decomposition is needed, because the configuration allows to set blocks
either only for writing or for both writing and reading.

### Executor

The `Executor` class emulates the Karma computer. It is an exported class
wrapping the methods of the `Impl` class. It is also used as the namespace
for all the classes described above.

The `Executor` class uses the
[pImpl](https://en.cppreference.com/w/cpp/language/pimpl) technique by storing
an `std::unique_ptr` to the instance of the `Impl` class created in
the constructor.

The exported methods of the `Executor` class accept a single compulsory
parameter specifying the path to the Karma executable file to be executed
as well as the following optional parameters:

* **Config**: an instance of the `Config` class specifying the configuration
  of the current execution, which is combined with the common configuration
  provided to the constructor of the `Executor` class (see [above](#config)
  for details)

* **Logger**: the output stream to print the execution process info, defaults
  to a no-op stream (i.e. the one that drops the messages instead of printing
  them)

> **Note**
>
> The overloads of the public methods of the `Executor` class are designed so
> that the **Config** optional parameter may be omitted if the default
> configuration is meant to be used, i.e. all the following calls are valid:
>
> ```c++
> karma::Executor executor; 
>
> executor.Execute(/* exec_path = */ "main.a",
>                  /* config = */ karma::Executor::Config::Strict(),
>                  /* log = */ std::clog);
> 
> executor.Execute(/* exec_path = */ "main.a",
>                  /* config = */ karma::Executor::Config::Strict());
> 
> executor.Execute(/* exec_path = */ "main.a",
>                  /* log = */ std::clog);
> 
> executor.Execute(/* exec_path = */ "main.a");
> ```
