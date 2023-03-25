# Formal task implementation

This file describes where to find the implementation of the parts of
the formal task to avoid any misunderstanding.

For details on how to compile and execute the Karma assembler programs mentioned
in this file, please refer to the playground directory
[README](playground/README.md).

### Part 1

The executor (the virtual machine) implementation can be found in
the [executor directory](include/executor).

The commands byte-code formats are described in the *Command formats*
section of the [docs](docs/Karma.pdf).

To get a program printing `"Hello, world!"` written in byte-code, one has to
compile a respective assembler file
(e.g. [this one](programs/playgrounds/hello_world_char_by_char.krm)).

### Part 2

The assembler commands are described in the *Command set* section of
the [docs](docs/Karma.pdf).

The compiler implementation can be found in
the [compiler directory](include/compiler).

The assembler program computing the n-th Fibonacci number using a loop can be
found [here](programs/playgrounds/fibonacci_loop.krm).

### Part 3

The `call` and `ret` commands descriptions can be found in
the *Function call commands* section of the [docs](docs/Karma.pdf).

The assembler program computing the n-th Fibonacci number using recursion can be
found [here](programs/playgrounds/fibonacci_recursion.krm).

### Part 4

The calling convention is described in the *Karma calling convention* section
of the [docs](docs/Karma.pdf).

The `printf` function is implemented in [printf.krm](programs/print/printf.krm)
as a part of the [printing library](programs/print).

> **Note**
> 
> Though the Karma assembler has an ability to print `uint32` and `double`
> values (via the `syscall` command), the printing library does not use that
> ability as described in its [README](programs/print/README.md).

The `printf` function usage samples can be found
[here](programs/playgrounds/printf.krm) (the sample from the task) and
[here](programs/print/playgrounds/printf.krm)
(the demonstration of all the supported formatting options).

### Part 5

The disassembler implementation can be found in
the [disassembler directory](include/disassembler).

