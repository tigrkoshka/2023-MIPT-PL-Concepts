# Types

## Overview

This directory provides a predefined exceptions hierarchy to be used
or derived from.

As the root of that hierarchy it provides the type `Exception`, which has
a special meaning in the *except* library: only those exception types, which
are derived from `Exception` are subject to upcast when checking the satisfaction
of a catch condition (see the *Type matching* section of the *except* library
[README](../README.md) for details).

## Dependencies

Both the declarations and the definitions of the symbols from this directory
are not dependent on any other parts of the [*except* library](..).

## Symbols

### Exported

```cpp
except::
      Exception
          RuntimeError
          |   RangeError
          |   OverflowError
          |   UnderflowError
          |   SystemError
          |
          LogicError
              InvalidArgument
              DomainError
              LengthError
              OutOfRange
```
