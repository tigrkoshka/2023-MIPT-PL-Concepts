## Exec

This directory provides the tools to read/write a Karma executable file,
which are used throughout the [karma library](../../include).

It also provides the types of the errors caused by an attempt to read a Karma
executable file, which are a part of the public interface of the
[karma library](../../include).

### Dependencies

Both the declarations and the definitions of the symbols from this directory
are dependent on and only on the symbols provided by
the [utils directory](../utils) and the [specs directory](../specs).

### Symbols

#### Public

```c++
karma::
        errors::                 // errors.hpp
                exec::
                        Error
                        ExecFileError
```

#### Internal

```c++
karma::
        Exec::                 // exec.hpp
                Data
                Write
                Read
```
