# Utils

This directory provides tools used for testing the *except* library.

Specifically, it provides:

* a struct `Recording` used to track the destructors calls during stack unwinding
* three types of custom exceptions (all derived from the `except::Exception`
  struct):
  * `CustomExceptionParent` is derived directly from `except::Exception`
  * `CustomExceptionChild` is derived from `CustomExceptionParent`
  * `CustomExceptionOther` is derived from `except::RuntimeError`

All the symbols are placed inside the `except::test::impl` namespace.

## Dependencies

Both the declarations and the definitions of the symbols from this directory
are not dependent on any other parts of the [*except* library](..).
