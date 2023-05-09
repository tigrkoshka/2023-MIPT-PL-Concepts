# Formal task implementation

This file describes where to find the implementation of the parts of
the formal task to avoid any misunderstanding.

### Destructors

The stack unwinding feature's main business logics is implemented inside
the [objects](include/objects) directory. The stack unwinding testing can
be found in [this](include/tests/suits/destructors.cpp) test file.

### Exceptions types

The description of the catch conditions can be found in the `CATCH` section
of the main [README](README.md). These conditions are implemented by the `Data`
struct in [this](include/impl/data.hpp) file. The testing of these conditions
can be found in [this](include/tests/suits/types.cpp) and
[this](include/tests/suits/catch.cpp) test files.
