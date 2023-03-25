# Printing library

This files of this directory implement the printing library.

## Concept

This library proves that the Karma assembler language no way more 'magical'
than the real-world assembler languages, i.e. that it does not abuse the fact
that it is being emulated by a C++ code and only uses some the higher-level
language features, such as arbitrary type formatting, purely for convenience
and not out of necessity.

## Non-cheating guarantee

The code in this library does not use the syscall codes that internally
implement the `uint32` or `double` values printing. In fact, the only syscall
the library uses is that with the code 105 (`PUTCHAR`). This can be checked by
searching for the following pattern inside this directory:

```regexp
syscall\s+r\d{1,2}\s+10[^5]
```

This pattern search produces some results for codes 100 (`SCANINT`) and
101 (`SCANDOUBLE`), but only inside the [playgrounds](playgrounds) directory,
where they are used to create interactive testing programs for the printing
library.

## Files

### Utility

The [base.krm](base.krm) file provides a base checking utility function used
throughout the printing library to validate the user-provided base and exit
the program with code 1 after printing an error message if the provided base is
invalid. The supported bases are from 2 to 36 inclusively.

The [digit.krm](digit.krm) file provides a function for printing a digit in
a number system of an arbitrary (but valid) base. The digit is printed as either
a decimal digit character or a lowercase latin letter. The supported bases range
guarantees that such any digit in a number system of a valid base can be
represented using those characters (since there are 10 + 26 = 36 of them, and
the maximal supported base is 36).

### Typed printing

The files named after types provide functions for printing values of
the respective types.

For all the types, except for `char` and `string` such files provide
a general-purpose function accepting additional arguments besides the value
itself (the base of the number system to output the value in or the double
value precision) as well as additional functions with some of those additional
parameters defaulted.

This imitates the functions overload and the optional parameters available
in some higher level languages (such as C++).

### Printf

The main function implemented by the library is the `printf` function defined
in the [printf.krm](printf.krm) file. The comments before the function and
at the beginning of the function document the function's arguments and
usage as well as all the available formatting options.

## Testing and usage samples

The [playgrounds](playgrounds) subdirectory provides sample programs using
the printing library aimed to test and demonstrate the functionality of
the library.

## Notes

The code in the files of this directory is heavily documented and is written
in a sustainable and scalable code style.
