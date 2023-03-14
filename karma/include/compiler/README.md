## Compiler

This directory provides the compiling part of the public interface
of the [karma library](../../include).

### Dependencies

Both the declarations and the definition of the symbols from
this directory are dependent on and only on the symbols provided by
the [utils directory](../utils), the [specs directory](../specs)
and the [exec directory](../exec).

### Symbols

#### Public

```c++
karma::
        compiler::                      // compiler.hpp
                MustCompile
                Compile
        errors::                        // errors.hpp
                compiler::
                        Error
                        InternalError
                        CompileError
```

#### Internal

```c++
karma::
        compiler::
                detail::
                        Entrypoint      // entrypoint.hpp
                        Labels          // labels.hpp
                        ExecData        // exec_data.hpp
                        File            // file.hpp
                        GetFiles        // includes.hpp
                        FileCompiler    // file_compiler.hpp
                        Impl            // impl.hpp
```
