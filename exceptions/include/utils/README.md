# Utils

## Overview

This directory provides basic metaprogramming utilities used throughout
the [*except* library](..).

## Dependencies

Both the declarations and the definitions of the symbols from this directory
are not dependent on any other parts of the [*except* library](..).

## Symbols

### Except internal

```cpp
except::detail::utils::
        concepts::                  // concepts.hpp
        |       Class
        |       NonCV
        |       NonCVRef
        |       Decayed
        |       ImplicitlyConstructibleFrom
        |       Throwable
        |       DecayedThrowable
        |       Catchable
        |
        type_traits::               // type_traits.hpp
                CatchType
```

### Internal

```cpp
except::detail::utils::concepts::TestImplicitConstruction
```
