# Disassembler

## Overview

This directory provides the disassembling part of the public interface
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

The `Labels` class is used to assign labels to the constants and certain places
of the code to make the disassembly (the result of disassembling) more
human-readable.

The labels assigned to the constants and the labels assigned to the instructions
are stored separately.

The constants labels are recoded using the `RecordConstantLabel` method that
accepts the address of the constants, assigns it a label and returns the name
of the assigned label.

The command labels are assigned using the `PrepareCommandLabels` method.
This method accepts the data extracted from the executable file
(see [below](#impl) for details) and does the following:

* Parses the code segment in search of `RM` and `J` type commands

* Parses those commands and extracts their address operand

* Skips the address and proceeds to the next command if one of
  the following is true:

    * the currently processed command is of `J` type and is contained by
      the `karma::detail::specs::cmd::kJIgnoreAddress` set (i.e. if the operand
      is ignored, see the [specs directory](../specs) for details)

    * the address operand is outside the code segment, i.e. it either
      refers to a constants (whose label should be assigned via
      the `RecordConstantLabel` method) or to a place in the memory allocated
      during runtime (i.e. such that there is no place in the disassembly
      to put a label to represent this address)

* If the conditions above do not apply, assigns a label to represent that
  address

* Additionally, assigns a predefined `main` label to the entrypoint specified
  by the provided executable file data

The `Labels` class also provides a `TryGetLabel` method, which searches for
a label assigned to either a constant or a command and returns its name if one
is found.

> **Note**
>
> The difference of the design chosen to assign labels to the constants
> versus to the commands is explained by the fact that we want to assign
> a label to each constant regardless of whether that label will be used
> somewhere in the code, but we only want to assign a label to an instruction
> if it will be used as another command's argument to make the disassembly more
> human-readable.
>
> Therefore, the constants segment needs to only be parsed once, and we can
> combine producing the labels for the constants with generating their
> declarations in the Karma assembler syntax. On the other hand, the code
> segment needs to be parsed twice: once to get all the commands that we want
> to assign labels to, and the second time to produce the commands in the Karma
> assembler syntax using said labels.
>
> Consequently, the constants labels recording is managed in the respective
> method of the `Impl` class (see [below](#impl) for details) and the `Labels`
> class only provides a method to generate such labels. With regard to
> the code segment, however, its first parsing is entirely decomposed into
> the `Labels` class method, and the `Impl` class methods only use the results
> it yields (see [below](#impl) for details).

### Impl

The `Impl` class provides the main part of the business logic of the Karma
executable file disassembling.

The public methods of this class do the following:

* Read the executable file via `Exec::Read` (see the exec directory
  [README](../exec/README.md) for details)

* Create a `Labels` class instance to assign labels to constants and certain
  commands (see [above](#labels) for details)

* Obtain the constants declarations by parsing the constants segment of
  the executable file. The types of the constants are determined based on
  the one-word prefix before their values (see the *Constants* section of
  the [docs](../../docs/Karma.pdf) for details). Each constant is assigned
  a label, which is recorded in the `Labels` class `RecordConstantLabel` method
  (see [above](#labels) for details)

* Call the `PrepareCommandLabels` method of the `Labels` class instance
  (see [above](#labels) for details)

* Obtain the commands representation in the Karma assembler syntax by parsing
  the code segment of the executable file. The commands are parsed via
  the functions in the `karma::specs::cmd::parse` namespace (see the specs
  directory [README](../specs/README.md) for details). If the current command
  address was assigned a label on the previous step, the label is put on
  a separate line and an additional newline is put before it

> **Note**
>
> For the `RM` and `J` type commands (except for the ones that ignore the
> address operand) a label is tried to be used instead of a numeric value for
> the address.
>
> The label is searched using the `Labels` class `TryGetLabel` method (see
> [above](#labels) for details).
>
> Due to the logic of the constants labels recording and
> the `PrepareCommandLabels` method of the `Labels` class (see [above](#labels)
> for details), this attempt succeeds if and only if the command's address
> operand is either inside the code or the constants segment.
>
> If the attempt failed, the address operand is written in a hexadecimal
> representation (unless it is ignored, in which case a decimal `0` value
> is written regardless of what the command's binary specified).

* Finishes the disassembled program with an `end main` directive

### Disassembler

The `Disassembler` class is an exported static class wrapping the methods
of the `Impl` class. It is also used as the namespace for all the classes
described above.

The exported methods of the `Disassembler` class accept a single compulsory
parameter specifying the path to the Karma executable file to be disassembled
as well as the following optional parameters:

* **Destination**: the destination to output the resulting Karma assembler code
  and, may be provided as:
    * the path of the resulting file
    * an `std::ostream`

  Defaults to a file in the same directory as the provided Karma executable
  file, with the same name, and with the last extension dropped and replaced
  with `_disassembled.krm`

* **Logger**: the output stream to print the disassembling process info,
  defaults to a no-op stream (i.e. the one that drops the messages instead of
  printing them)

> **Note**
>
> The overloads of the public methods of the `Disassembler` class are designed
> so that the **Destination** optional parameter may be omitted if
> the default destination is meant to be used, i.e. all the following calls
> are valid:
>
> ```c++
> karma::Disassembler::Disassemble(/* src = */ "main.a",
>                                  /* dst = */ std::cout, 
>                                  /* log = */ std::clog);
> 
> karma::Disassembler::Disassemble(/* src = */ "main.a",
>                                  /* dst = */ std::cout);
> 
> // need to explicitly wrap the logger in a karma::Logger struct,
> // otherwise the overload above will be preferred,
> // and the provided stream will be used as the output
> // destination instead of the logs destination
> karma::Disassembler::Disassemble(/* src = */ "main.a",
>                                  /* log = */ karma::Logger(std::clog));
> 
> karma::Disassembler::Disassemble(/* src = */ "main.a");
> ```
