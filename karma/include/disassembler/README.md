# Disassembler

## Overview

This directory provides the disassembling part of the public interface
of the [karma library](..).

## Dependencies

The declarations of the symbols from this directory are dependent on and only on
the symbols provided by the [utils directory](../utils)
and the [specs directory](../specs).

The definitions are additionally dependent on the symbols provided
by the [exec directory](../exec).

## Symbols

### Public

```c++
karma::                                     // disassembler.hpp
        Disassembler::
        |       MustDisassemble
        |       Disassemble
        |
        errors::
                disassembler::
                        Error
                        InternalError
                        DisassembleError
```

### Internal

```c++
karma::
        Disassembler::Impl                  // impl.hpp
        |
        errors::                            // errors.hpp
                disassembler::
                        InternalError::Builder
                        DisassembleError::Builder
```

## Design description

### Impl

The `Impl` class provides the business logic of the Karma executable file
decompiling.

The public methods of this class do the following:

* Reads the executable file via `Exec::Read` (see the exec directory
  [README](../exec/README.md) for details)

* Obtains the constants declarations by parsing the constants segment of
  the executable file. The types of the constants are determined based on
  the one-word prefix before their values (see the *Constants* segment of
  the [docs](../../docs/Karma.pdf) for details)

* Obtains the commands representation in the Karma assembler syntax by parsing
  the code segment of the executable file. The commands are parsed via
  the functions in the `karma::specs::cmd::parse` namespace (see the specs
  directory [README](../specs/README.md) for details).

* Assigns a `main` label to the entrypoint address, i.e. places it before the
  command that has the address specified as the entrypoint by the contents of
  the executable file

* Finishes the disassembled program with an `end main` statement

### Disassembler

The `Disassembler` class is an exported static class simply wrapping the methods
of the `Impl` class. It is also used as the namespace for the `Impl` class.
