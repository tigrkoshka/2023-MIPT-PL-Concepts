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
        Disassembler:
        |       Labels                      // labels.hpp
        |       Impl                        // impl.hpp
        |
        errors::                            // errors.hpp
                disassembler::
                        InternalError::Builder
                        DisassembleError::Builder
```

## Design description

### Labels

The `Labels` class is used to

### Impl

The `Impl` class provides the business logic of the Karma executable file
decompiling.

The public methods of this class do the following:

* Reads the executable file via `Exec::Read` (see the exec directory
  [README](../exec/README.md) for details)

* Creates a `Labels` class instance to assign labels to constants and certain
  commands (see [above](#labels) for details)

* Obtains the constants declarations by parsing the constants segment of
  the executable file. The types of the constants are determined based on
  the one-word prefix before their values (see the *Constants* segment of
  the [docs](../../docs/Karma.pdf) for details). Each constant is assigned
  a label, which is recorded in the `Labels` class instance

* Calls the `PrepareCommandLabels` method of the `Labels` class instance
  (see [above](#labels) for details)

* Obtains the commands representation in the Karma assembler syntax by parsing
  the code segment of the executable file. The commands are parsed via
  the functions in the `karma::specs::cmd::parse` namespace (see the specs
  directory [README](../specs/README.md) for details). If the current command
  address was assigned a label on the previous step, the label is put on
  a separate line and an additional newline is put before it

> **Note**
>
> For the `RM` and `J` type commands (except for the ones that ignore the
> address operand) a label is tried to be used instead of a numeric value for
> the address. The label is searched among the ones recorded in the `Labels`
> class instance. Due to the constants labels recording and
> the `PrepareCommandLabels` method of the `Labels` class instance logics
> (see [above](#labels) for details), this attempt succeeds if and only if the
> command's address operand is either inside the code or the constants segment.
> If the attempt failed, the address operand is written in a hexadecimal
> representation.

* Finishes the disassembled program with an `end main` statement

### Disassembler

The `Disassembler` class is an exported static class simply wrapping the methods
of the `Impl` class. It is also used as the namespace for all the classes
described above.
