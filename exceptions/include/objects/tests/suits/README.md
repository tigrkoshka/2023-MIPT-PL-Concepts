# Suits

This directory provides the tests for the `AutoObject` class template. Besides
the testing itself, these tests aim to be used as samples of the `AutoObject`
class template usage and are considered a part of the documentation. As such,
their only dependency outside the [tests directory](..) is
the [`AutoObject`](../../auto.hpp) class template.

All the tests provided by this directory are placed inside
the `except::test::objects` namespace.

## Fixture

Since the `AutoObject` class template has two specializations (for wrapping 
`final` and for non-`final` classes) and it aims to be used as indifferently
as possible from the class it wraps, the same tests need to be run twice:
for a `final` and for a non-`final` wrapped class. These classes are provided 
by the [utils directory](../utils).

To run the tests twice, the `TYPED_TEST` macro of the Google Test framework 
is used. The fixture class template is defined in the [fixture.hpp](fixture.hpp)
file. All the tests, except for the ones testing the `AutoObject`
[template](templates.cpp) itself, are run using this fixture.

For more details on typed tests please refer to the Google Test
[documentation](https://clck.ru/34N5Dt).

## Tests

The files inside this directory are named after the features of the `AutoObject`
class template they test.
