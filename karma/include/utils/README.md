# Utils

This directory provides basic utility functions used throughout
the [karma library](../../include).

It also provides the base type for all [karma library](../../include) error,
which is a part of the public interface of the [karma library](../../include).

## Dependencies

Both the declarations and the definitions of the symbols from this directory
are not dependent on any other parts of the [karma library](../../include).

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
