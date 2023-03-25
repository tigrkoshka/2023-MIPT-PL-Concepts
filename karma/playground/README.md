# Playground

This directory provides a sample C++ program using
the [*karma* library](../include).

## Programs

The Karma assembler programs to be compiled and executed can be found in
the [programs](../programs) directory. Alternatively, one can write their own
Karma assembler program and test the [*karma* library](../include) on it.

## Produced files

After compilation, if the default binary destination was used, the executable
file can be found in the same directory as the original Karma assembler file
with extension `.a`.

If that file is then disassembled and the default destination is used for that
as well, the disassembled Karma program can be found in the same directory
as the initial file with a `_disassembled` suffix and extension `.krm`.

For details on default output files destinations refer to
the compiler directory [README](../include/compiler/README.md) and
the disassembler directory [README](../include/disassembler/README.md).
