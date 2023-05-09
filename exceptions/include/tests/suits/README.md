# Suits

This directory provides the tests for the *except* library. Besides the testing
itself, these tests aim to be used as samples of the *except* library usage and
are considered a part of the documentation. As such, they always include
the *except* library via the `<except>` header (as it would be done when using
the library) and only use the exported macros and symbols of
the *except* library.

All the tests provided by this directory are placed inside
the `except::test::impl` namespace.

## Tests

The tests are divided into several files, each of which provide a single test
suite testing a specific feature of the *except* library.

The suites are listed in the following table:

| File                               | Suite name  | The tested feature                        |
|------------------------------------|-------------|-------------------------------------------|
| [simple.cpp](simple.cpp)           | Simple      | General usage of the *except* library     |
| [types.cpp](types.cpp)             | Types       | Catching different types of exceptions    |
| [catch.cpp](catch.cpp)             | Catch       | Catch options (by value/by reference/all) |
| [destructors.cpp](destructors.cpp) | Destructors | Stack unwinding                           |
| [death.cpp](death.cpp)             | Death       | Program termination                       |

## General usage

This section provides the descriptions of the tests from the [Simple](simple.cpp)
suite.

For each test case a code snippet is provided. The code itself is the same as
the one in the actual test except that the lines not using the *except* library
(e.g. Google Test assertions) are omitted and the *except* exported library
macros and functions are replaced with C++ native keywords to enable better
syntax highlighting (the *except* library is designed so that an obvious
one-to-one correspondence exists between the two).

To the right of the code a table is provided as a comment. The table describes
what is happening during the code execution. It has two columns: `Stack` and
`Caught`, which correspond to the two stacks used internally by the *except*
library (see the impl directory [README](../../impl/README.md) for details).

A row may not be filled if no changes are made to any of the stacks' contents by
the respective line. If it is filled, it shows the contents of the stacks *after*
the respective line has been executed. The rows corresponding to the closing
parenthesis of a `try-catch` block represent the state of the stacks after the
execution has exited the block. If the exit from the `try-catch` block is not
performed in a regular way (i.e. if an exception has been thrown from the `catch`
block), its closing parenthesis is marked as `SKIPPED`.

In each column contains the list of exceptions, which are in the respective stack
at the current point of execution. The exceptions are written in the order of
their addition to the corresponding stack (i.e. the top of the stack comes last).
Each exception record has the form `<id>[*] (<depth>)`, where:

* `<id>` is a unique numerical identifier of the exception, which is not actually
  stored during execution, but is present in the table to clearly distinguish
  the exceptions

* an optional `*` character is assigned to an exception in the `Stack` column
  if it has been raised but not yet caught (during runtime, such exceptions
  are still inside the `stack` and not the `caught` stack)

* `<depth>` is the depth of `stack` when the exception was first put there
  (this property does not ever change for each exception)

To the right of the table there are additional descriptions for some rows,
which further explain what is happening in the respective line of the code.
In these explanations the exceptions are referred to by their `<id>` values
in square brackets. An `<id>` may be followed by one or more `^` characters,
which indicates that which the id of the exception on which the actual actions
are performed may be different from the specified one, its contents are
those of the one with the specified id, i.e. that the action is performed on
a rethrown exception. The number of `^` characters indicates how many times
the initial exception was rethrown before it got to the current point.

For example, the comment `catches [2^^]` means that the exception with id `2`
was thrown at some point before the current line, then it was caught, rethrown,
caught again and rethrown again before it was caught in the current line.

Note that a rethrow may either occur explicitly with the `throw;` command
or implicitly at the end of a `try-catch` block if the initially thrown exception
did not satisfy any of the provided catch conditions. In both cases
an explanatory comment is provided to the right of the table.

### Case 1

```cpp
                                     // |        Stack         |    Caught    |
                                     // |----------------------|--------------|
try {                                // | 1 (0)                |              |
    try {                            // | 1 (0), 2 (1)         |              |
       throw RuntimeError{};         // | 1 (0), 2* (1)        |              | throws [2]
    } catch (RuntimeError) {         // | 1 (0)                | 2 (1)        | catches [2]
       try {                         // | 1 (0), 3 (1)         | 2 (1)        |
          try {                      // | 1 (0), 3 (1), 4 (2)  | 2 (1)        |
             throw;                  // | 1 (0), 3 (1), 4* (2) | 2 (1)        | rethrows [2]
          } catch (RuntimeError) {   // | 1 (0), 3 (1)         | 2 (1), 4 (2) | catches [2^]
             throw;                  // | 1 (0), 3* (1)        | 2 (1)        | rethrows [2^]
          }                          //                     SKIPPED
       } catch(RuntimeError) {       // | 1 (0)                | 2 (1), 3 (1) | catches [2^^]
          // OK                      // |                      |              |
       }                             // | 1 (0)                | 2 (1)        |
       throw;                        // | 1* (0)               |              | rethrows [2]
    }                                //                     SKIPPED
} catch (RuntimeError) {             // |                      | 1 (0)        | catches [2^]
    // OK                            // |                      |              |
}                                    // |                      |              |
```

### Case 2

```cpp
                                 // |         Stack        |    Caught    |
                                 // |----------------------|--------------|
try {                            // | 1 (0)                |              |
    try {                        // | 1 (0), 2 (1)         |              |
        throw RuntimeError{};    // | 1 (0), 2* (1)        |              | throws [2]
    } catch (RuntimeError) {     // | 1 (0)                | 2 (1)        | catches [2]
        try {                    // | 1 (0), 3 (1)         | 2 (1)        |
            throw LogicError{};  // | 1 (0), 3* (1)        | 2 (1)        | throws [3]
        } catch (LogicError) {   // | 1 (0)                | 2 (1), 3 (1) | catches [3]
            throw;               // | 1* (0)               |              | rethrows [3]
        }                        //                     SKIPPED
    }                            //                     SKIPPED
} catch (LogicError) {           // |                      | 1 (0)        | catches [3^]
    // OK                        // |                      |              |
}                                // |                      |              |
```

### Case 3

```cpp
                               // |         Stack        | Caught |
                               // |--------------------- |--------|
try {                          // | 1 (0)                |        |
    try {                      // | 1 (0), 2 (1)         |        |
        throw RuntimeError{};  // | 1 (0), 2* (1)        |        | throws [2]
    } catch (RuntimeError) {   // | 1 (0)                | 2 (1)  | catches [2]
        throw LogicError{};    // | 1* (0)               |        | throws [1]
    }                          //                     SKIPPED
} catch (LogicError) {         // |                      | 1 (0)  | catches [1]
    // OK                      // |                      |        |
}                              // |                      |        |
```

### Case 4

```cpp
                                           // |            Stack            |    Caught    |
                                           // |-----------------------------|--------------|
try {                                      // | 1 (0)                       |              |
    try {                                  // | 1 (0), 2 (1)                |              |
        try {                              // | 1 (0), 2 (1), 3 (2)         |              |
            try {                          // | 1 (0), 2 (1), 3 (2), 4 (3)  |              |
                throw RuntimeError{};      // | 1 (0), 2 (1), 3 (2), 4* (3) |              | throws [4]
            } catch (RuntimeError) {       // | 1 (0), 2 (1), 3 (2)         | 4 (3)        | catches [4]
                throw LogicError{};        // | 1 (0), 2 (1), 3* (2)        |              | throws [3]
            }                              //                            SKIPPED
        } catch (LogicError) {             // | 1 (0), 2 (1)                | 3 (2)        | catches [3]
            throw;                         // | 1 (0), 2* (1)               |              | rethrows [3]
        }                                  //                            SKIPPED
    } catch (LogicError) {                 // | 1 (0)                       | 2 (1)        | catches [3^]
        try {                              // | 1 (0), 5 (1)                | 2 (1)        |
            throw RuntimeError{};          // | 1 (0), 5* (1)               | 2 (1)        | throws [5]
        } catch (RuntimeError) {           // | 1 (0)                       | 2 (1), 5(1)  | catches [5]
            // OK                          // |                             |              |
        }                                  // | 1 (0)                       | 2 (1)        |
        throw;                             // | 1* (0)                      |              | rethrows [3^]
    }                                      //                            SKIPPED
} catch (LogicError) {                     // |                             | 1 (0)        | catches [3^^]
    // OK                                  // |                             |              |
}                                          // |                             |              |
```

### Case 5

```cpp
                                           // |            Stack            |    Caught    |
                                           // |-----------------------------|--------------|
try {                                      // | 1 (0)                       |              |
     try {                                 // | 1 (0), 2 (0)                |              |
         throw RuntimeError{};             // | 1 (0), 2* (1)               |              | throws [2]
     } catch (RuntimeError) {              // | 1 (0)                       | 2 (1)        | catches [2]
         try {                             // | 1 (0), 3 (1)                | 2 (1)        |
             try {                         // | 1 (0), 3 (1), 4 (2)         | 2 (1)        |
                 try {                     // | 1 (0), 3 (1), 4 (2), 5 (3)  | 2 (1)        |
                     throw LogicError{};   // | 1 (0), 3 (1), 4 (2), 5* (3) | 2 (1)        | throws [5]
                 } catch (LogicError) {    // | 1 (0), 3 (1), 4 (2)         | 2 (1), 5 (3) | catches [5]
                     throw;                // | 1 (0), 3 (1), 4* (2)        | 2 (1)        | rethrows [5]
                 }                         // |                             |              |
             } catch (RuntimeError) {      // |                             |              | fails to catch [5^]
                 // do not enter           // |                             |              |
             }                             // | 1 (0), 3* (1)               | 2 (1)        | rethrows [5^]
         } catch (RuntimeError) {          // |                             |              | fails to catch [5^^]
            // do not enter                // |                             |              |
         }                                 // | 1* (0)                      |              | rethrows [5^^]
     }                                     //                            SKIPPED
 } catch (LogicError) {                    // |                             | 1 (0)        | catches [5^^^]
     // OK                                 // |                             |              |
 }                                         // |                             |              |
```

### Case 6

```cpp
                           // | Stack | Caught |
                           // |-------|--------|
try {                      // | 1 (0) |        |
   // OK                   // |       |        |
} catch (RuntimeError) {   // |       |        | nothing to catch
   // do not enter         // |       |        |
}                          // |       |        |
```

### Case 7

```cpp
                             // | Stack  | Caught |
                             // |--------|--------|
try {                        // | 1 (0)  |        |
   throw RuntimeError{};     // | 1* (0) |        | throws [1]
} catch (RuntimeError) {     // |        | 1 (0)  | catches [1]
   try {                     // | 2 (0)  | 1 (0)  |
       // OK                 // |        |        |
   } catch (LogicError) {    // | 2 (0)  | 1 (0)  | nothing to catch
       // do not enter       // |        |        |
   }                         // |        | 1 (0)  |
}                            // |        |        |
```

### Case 8

```cpp
                                     // |             Stack            | Caught |
                                     // |------------------------------|--------|
try {                                // | 1 (0)                        |        |
    try {                            // | 1 (0), 2 (1)                 |        |
        try {                        // | 1 (0), 2 (1), 3 (2)          |        |
            try {                    // | 1 (0), 2 (1), 3 (2), 4 (3)   |        |
                throw Exception{};   // | 1 (0), 2 (1), 3 (2), 4* (3)  |        | throws [4]
            } catch(Exception) {     // | 1 (0), 2 (1), 3 (2)          | 4 (3)  | catches [4]
                throw;               // | 1 (0), 2 (1), 3* (2)         |        | rethrows [4]
            }                        //                             SKIPPED
        } catch(Exception) {         // | 1 (0), 2 (1)                 | 3 (2)  | catches [4^]
            // OK                    // |                              |        |
        }                            // | 1 (0), 2 (1)                 |        |
        throw Exception{};           // | 1 (0), 2* (1)                |        | throws [2]
    } catch(Exception) {             // | 1 (0)                        | 2 (1)  | catches [2]
        try {                        // | 1 (0), 5 (1)                 | 2 (1)  |
            // OK                    // |                              |        |
        } catch(...) {               // | 1 (0), 5 (1)                 | 2 (1)  | nothing to catch
            // do not enter          // |                              |        |
        }                            // | 1 (0)                        | 2 (1)  |
    }                                // | 1 (0)                        |        |
    throw Exception{};               // | 1* (0)                       |        | throws [1]
} catch(Exception) {                 // |                              | 1 (0)  | catches [1]
    // OK                            // |                              |        |
}                                    // |                              |        |
```
