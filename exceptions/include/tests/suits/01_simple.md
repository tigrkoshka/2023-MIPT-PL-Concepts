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
} catch (RuntimeError) {             // |                      | 1 (0)        | catches
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
} catch (LogicError) {           // |                      | 1 (0)        | catches [3]
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
 } catch (LogicError) {                    // |                             | 1 (0)        | catches [5^^]
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