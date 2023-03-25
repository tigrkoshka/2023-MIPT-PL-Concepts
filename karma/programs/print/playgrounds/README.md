# Playgrounds

This directory provides sample programs using the printing library.

The files named after the types that can be printed using the printing library
provide simple programs mainly used for the printing library testing.
Some of these programs are interactive, while others simply print predefined
constants.

That is due to the fact that the standard-provided user input management options
are limited, and do not include obtaining 64-bit integral values from `stdin`.

> **Note**
>
> The user input management logic can potentially be implemented entirely
> on the Karma assembler language with using only the 104 (`GETCHAR`) syscall
> code.
>
> However, that is a task of a different library (probably named
> *the scanning library* with the main formatted input parsing function
> `scanf`), and implementing such a library is out of the `karma` project scope.

The [printf.krm](printf.krm) file provides a program that uses the full power
of the `printf` function and prints all kinds of predefined values as samples
of the formatting options usage.
