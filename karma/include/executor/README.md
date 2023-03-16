## Executor

This directory provides the execution part of the public interface
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
        Executor::                      // executor.hpp
        |       MustExecute
        |       Execute
        |       Config                  // config.hpp
        |
        errors::                        // executor.hpp
                executor::
                        Error
                        InternalError
                        ExecutionError
```

#### Internal

```c++
karma::
        Executor::
        |       Storage              // storage.hpp
        |       CommandExecutor      // command_executor.hpp
        |       CommonExecutor       // common_executor.hpp
        |       RMExecutor           // rm_executor.hpp
        |       RRExecutor           // rr_executor.hpp
        |       RIExecutor           // ri_executor.hpp
        |       JExecutor            // j_executor.hpp
        |       Impl                 // impl.hpp
        |               
        errors::                     // errors.hpp
                executor::
                        InternalError::Builder
                        ExecutionError::Builder
```
