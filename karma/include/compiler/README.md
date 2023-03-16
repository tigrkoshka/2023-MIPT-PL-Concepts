# Compiler

This directory provides the compiling part of the public interface
of the [karma library](../../include).

## Dependencies

Both the declarations and the definition of the symbols from
this directory are dependent on and only on the symbols provided by
the [utils directory](../utils), the [specs directory](../specs)
and the [exec directory](../exec).

## Symbols

### Public

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
        |       ExecData                // exec_data.hpp
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
