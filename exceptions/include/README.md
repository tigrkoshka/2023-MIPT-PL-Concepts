# Except library

## Overview

The *except* library provides a custom implementation of C++ exceptions handling.
The features provided by the *except* library can be divided into three groups:

* preprocessor macros `TRY` and `CATCH` and the function `Throw` aimed to replace
  the C++ keywords `try`, `catch` and `throw` respectively

* class template `AutoObject`, which must be explicitly used for automatic stack
  unwinding when throwing an exception

* the predefined exceptions hierarchy to be used and/or derived from

Since the *except* library seeks to provide a (somewhat incomplete) alternative
to the C++ native exceptions handling, it is designed so that its usage is
as close as possible to that of the native C++ keywords
(see [below](#usage) for details).

## Exported symbols

```c++
#define TRY ...
#define CATCH ...

::
   Throw(...)
   AutoObject
   |
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

> **Note**
>
> Since the main usage of the *except* library happens via preprocessor macros,
> i.e. with code generation, the *except* library has no choice but to export its
> internal symbols (since they are to be compiled directly from the user's code).
>
> To avoid confusion, all the internal exported *except* library preprocessor
> macros start from an `EXCEPT_DETAIL` prefix, and all the symbols are placed
> inside the namespace `except::detail`. Such macros and symbols are not meant
> for direct use from the user's code. Any violation of this rule
> (i.e. direct usage of the *except* library implementation details) leads
> to undefined behaviour.

## Usage

### TRY

The `TRY` preprocessor macro imitates the C++ `try` keyword. Thus, it does not
accept any arguments and must be followed by a block of code inside curly
brackets (which may be omitted if the block consists of a single statement).

### CATCH

The `CATCH` preprocessor macro imitates the C++ `catch` keyword.
Thus, it may accept 0, 1 or 2 arguments (see [below](#arguments) for explanation)
and must be followed by a block of code inside curly brackets (which may be
omitted if the block consists of a single statement).

A `CATCH` macro must appear immediately after the block of code following
either a `TRY` macro or another `CATCH` macro.

The `CATCH` macros following the same `TRY` block form a *catch sequence*.
The arguments of each `CATCH` macro represent a *catch condition*
(see [below](#arguments) for details).

If an exception is thrown inside a `TRY` block (via the `Throw` function,
see [below](#throwing) for details), the catch conditions of the respective
sequence are checked in the order of appearance, and *only* the block of code
following the first satisfied condition is executed (all the blocks following
other `CATCH` macros of the conditions, both before and after the executed one,
are not entered).

#### Arguments

If the `CATCH` macro accepts 0 arguments (the round brackets are still needed
since it is a functional style macro), its catch condition is always true
(i.e. the respective block of code is always executed if no previous catch
conditions were satisfied).

If the `CATCH` macro accepts 1 argument, it must be a (possibly cv-qualified)
typename (possibly an lvalue reference). In this case the block is executed if
the type of the exception thrown in the respective `TRY` block *matches*
the provided type (see [below](#type-matching) for details).

> **Note**
> 
> The type passed to the `CATCH` cannot be an rvalue reference or
> a (possibly cv-qualified) abstract class (if it is, the code will not compile).
> 
> If the type passed to the `CATCH` macro is an array or a function,
> it is treated as the respective pointer type.

If the `CATCH` macro accepts 2 arguments, the first one has the same semantics
as the only argument from the previous case, and the second one is the name,
which will be assigned to the caught exception (either by value or by reference,
possibly cv-qualified, depending on the first argument).

> **Note**
>
> If an exception is caught by a non-const lvalue reference, all actions
> performed on it are applied directly on the thrown instance.
> 
> This can be used to catch an exception, make some changes to it
> (e.g. edit its message) and rethrow it (see [below](#rethrowing) for details).

#### Type matching

Let the type of the initially thrown exception be `A` and the type provided
to the `CATCH` macro be `B`. 

The catch condition is always satisfied if
[`std::decay_t<A>`](https://clck.ru/34MqP6) is the same as
[`std::decay_t<B>`](https://clck.ru/34MqP6).

If `A` is derived from `except::Exception`, the catch condition is also satisfied
if one of the following is true:

* `B` is an unambiguous public base class of `A`
* `B` is an lvalue reference to a (possibly cv-qualified)
  unambiguous public base class of `A`

> **Note**
> 
> The current implementation does not support standard pointer conversion or
> qualification conversion.
> 
> This means, for example, that if the initially thrown exception was of type
> `char*` and the type passed to the `CATCH` macro is `const char*`, the catch
> condition is not satisfied (i.e. pointer qualification conversion is not
> supported). However, if the type passed to the `CATCH` macro is `char* const`,
> the catch condition is satisfied, because `std::decay_t<char* const>`
> is `char*`.
> 
> Another example is if the initially thrown exception was of type
> `except::RuntimeError*` and the type passed to the `CATCH` macro
> is `except::Exception*`, the catch condition is not satisfied.

#### Uncaught exception

If no catch conditions from the catch sequence are satisfied, `std::terminate`
is called. It is unspecified whether stack unwinding 
(see [below](#stack-unwinding) for details) is performed, throwing an uncaught
exception is permitted to terminate the program without invoking any destructors.

### Throw

The `Throw` function imitates the C++ `throw` keyword. Thus, it may accept
0 or 1 argument.

#### Throwing

If the overload of the `Throw` function accepting an argument is used, it does
the following:

* Initializes (either with a copy or a move) the exception data with the provided
  argument. The argument must be both copy-constructible and destructible
  (otherwise, a compilation error occurs)

* Performs stack unwinding (see [below](#stack-unwinding) for details)
  up to the enclosing `TRY` block

* Begins the search for a matching `CATCH` block (i.e. the one for which
  the catch condition is satisfied with respect to the type of the thrown
  exception data)

#### Rethrowing

If the overload of the `Throw` function accepting 0 arguments is used, 
it rethrows the exception that is currently being processed. This overload may
only be used inside a `CATCH` block. Its use outside a `CATCH` block results
in a call to `std::terminate`. 

The rethrown exception is copy-initialized from the currently processed exception
(regardless of the type by which it was caught, i.e. if an exception of type
`except::RuntimeError` was caught by passing `const except::Exception&` to
the `CATCH` macro and then rethrown, it can be caught again by passing 
`const except::RuntimeError&` to the respective `CATCH` macro).

The rethrown exception cannot be caught by any `CATCH` macros from the same
catch sequence as the one it was rethrown from. To catch a rethrown exception,
a separate `TRY` block must be present enclosing the current catch sequence
as well as the `TRY` block the sequence follows.

### Stack unwinding

When an exception is thrown, all the objects that were placed on the stack
between the beginning of the most narrow enclosing `TRY` block and the call to
the `Throw` function, are destroyed in the reverse order of their creation.
That includes the function local variables and arguments that were passed
by value for all the functions on the stack between the call to `Throw` and
the most narrow enclosing `TRY` block.

#### AutoObject

For the stack unwinding to work correctly, all objects created on the stack
in the potentially throwing environment must be stored inside an instance
of the `AutoObject` class template.

Since stack unwinding calls the destructors, only non cv-qualified classes
are permitted as template arguments to the `AutoObject`:

* the non-class variable need not be wrapped in `AutoObject` since they
do not have non-trivial destructors
* the cv-qualifiers, if desired, must refer to the `AutoObject` rather than
to its template parameter
* if no object is created (e.g. a function returns an lvalue reference or
an argument is passed to a function by reference) there is no need to use
`AutoObject` since no objects are being constructed

The `AutoObject` class template is designed so that its usage is as close as
possible to the usage of the instance stored inside of it:

* it is implicitly constructible from and convertible to the stored
  class instance
* it supports all the same constructors and operators that the stored class does
* the public fields and methods of the stored class instance are always
  accessible via `operator->` and are also accessible via `operator.` if
  the stored class is not declared `final` (in this case `AutoObject` is simply
  derived from the stored class)

> **Note**
> 
> If the stored class is declared `final` a problem arises for the overload
> resolution when a function can accept both `const T&` and `T&&`. To resolve
> the conflict, in such cases there is an rvalue method `Move()` defined
> for `AutoObject<T>`, which forces the overload accepting `T&&`.
> 
> For a sample of such a case and the `Move()` method usage please refer to
> the last section of [this test](objects/tests/suits/07_usage.cpp).

For more details on `AutoObject` implementation and usage please refer to
the objects directory [README](objects/README.md).
