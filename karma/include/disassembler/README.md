## Disassembler

This directory provides the disassembling part of the public interface
of the [karma library](../../include).

### Dependencies

The declarations of the symbols from this directory are dependent on and only on
the symbols provided by the [utils directory](../utils)
and the [specs directory](../specs).

The definitions are additionally dependent on the symbols provided
by the [exec directory](../exec).

### Symbols

#### Public

```c++
karma::
        Disassembler::                      // disassembler.hpp
                MustDisassemble
                Disassemble
        errors::                            // errors.hpp
                disassembler::
                        Error
                        InternalError
                        DisassembleError
```

#### Internal

```c++
karma::
        Disassembler::
                Impl     // impl.hpp 
```
