# Exec

## Overview

This directory provides the tools to read/write a Karma executable file,
which are used throughout the [*karma* library](..).

It also provides the types of the errors caused by an attempt to read a Karma
executable file, which are a part of the public interface of the
[*karma* library](..).

## Dependencies

Both the declarations and the definitions of the symbols from this directory
are dependent on and only on the symbols provided by
the [utils directory](../utils) and the [specs directory](../specs).

## Symbols

### Karma internal

```c++
karma::                        // exec.hpp
        Exec::
                Data
                Write
                Read
                
karma::
        errors::
                exec::
                        Error
                        ExecFileError
```

### Internal

```c++
karma::                        // errors.hpp
        errors::
                exec::
                        ExecFileError::Builder
```
