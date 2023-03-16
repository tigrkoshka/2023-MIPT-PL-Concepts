# Karma library

The karma library provides tools to compile Karma assembler files and
execute the resulting Karma executable file as well as to disassemble
an existing Karma executable file back into the Karma assembler language.

Thus, the karma library consists of three _blocks_: `compiler`, `executor` and
`disassembler`.

## Exported symbols

```c++
karma::
        Compiler::                       // compiler block
                MustCompile
                Compile

        Executor::                       // executor block
                MustExecute
                Execute
                Config
                
        Disassembler::                   // disassembler block
                MustDisassemble
                Disassemble
                
        errors::
                Error
                
                compiler::               // compiler block
                        Error
                        InternalError
                        CompileError
                        
                executor::               // executor block
                        Error
                        InternalError
                        ExecutionError
                        
                disassembler::           // disassembler block
                        Error
                        InternalError
                        DisassembleError
```

## Methods

All exported methods od the karma library classes have two variants,
one of which has a `Must` prefix, and the other does not.

The former throw exceptions (see [below](#exceptions)),
while the latter wrap them in a `try-catch` block and print the exception
info to `stdout`.

## Exceptions

It is guaranteed that any exception thrown by the public methods of the karma
library is either of type `karma::errors::Error` or of a type derived
from `karma::errors::Error`.

Note that this does not guarantee that any exception thrown
by the public methods of the karma library has a type listed above,
just that it can be caught as a `karma::errors::Error`.

Therefore, an exception-safe karma library usage either only
uses exported methods without the `Must` prefix, or looks somewhat like this:

```c++
try {
    karma::Compiler::MustCompile(karma_assembler_file_path);
} catch (const karma::errors::compiler::CompileError& e) {
    // handle compile-time error of the provided assembler code
} catch (const karma::errors::compiler::InternalError& e) {
    // handle internal compiler error
} catch (const karma::errors::compiler::Error& e) {
    // handle an unexpected compiler error
} catch (const karma::errors::Error& e) {
    // handle an error not directly related to the compiler
}
```

or simply:

```c++
try {
    karma::Compiler::MustCompile(karma_assembler_file_path);
} catch (const karma::errors::Error& e) {
    // handle any possible error
}
```