# Utils

This directory provides tools used for testing the `AutoObject` class template.

Specifically it provides:

* concepts used for static feature testing, which sometimes repeat the concepts
  from the *except* library [utils](../../../utils) directory to avoid
  unnecessary dependencies

* the `Stats` struct used to keep track of the number and types of objects
  constructions, destructions and assignments

* the operators for `Stats` printing used to get better testing log messages
  on assertions failures

* the function `ResetStats` used to obtain objects constructions, destructions
  and assignments statistics separately for each test

* the `Simple` and `Final` struct templates which imitate the user-defined
  classes to be wrapped in the `AutoObject` class template

The `Simple` and `Final` struct templates are exactly the same except that
the latter is declared `final`. Each of them has two boolean template
parameters which are used to make their constructors/methods `explicit`
and/or `noexcept`. Convenient type definitions are provided for instantiations
of these struct templates with different combinations of their template
arguments. All the instantiations names are prefixed with the respective
struct template name.

All the symbols are placed inside the `except::test::objects` namespace.

## Dependencies

Both the declarations and the definitions of the symbols from this directory
are only dependent on the [`AutoObject`](../../auto.hpp) class template.
