# Utils

## Overview

This directory provides basic utility functions used throughout
the [karma library](..).

It also provides the base type for all [karma library](..) errors,
which is a part of the public interface of the [karma library](..).

## Dependencies

Both the declarations and the definitions of the symbols from this directory
are not dependent on any other parts of the [karma library](..).

## Symbols

### Public

```c++
karma::
        errors::       // error.hpp
                Error
```

### Karma internal

```c++
karma::
        detail::
                utils::
                        concepts::                   // concepts.hpp
                        |        IntOrFloat
                        |        Hashable
                        logger::                     // logger.hpp
                        |        no_op
                        map::                        // map.hpp
                        |        Revert
                        strings::                    // strings.hpp
                        |        TrimSpaces
                        |        Escape
                        |        Unescape
                        traits::                     // traits.hpp
                        |        NonCopyableMovable
                        |        Static
                        types::                      // types.hpp
                        |        kByteSize
                        |        Split
                        |        Join
                        |        GetSignedValue
                        |        GetUnsignedBits
                        vector::                     // vector.hpp
                        |        Append
                        |        CopyToBegin
                        |    
                        Generator                    // generator.hpp

```
