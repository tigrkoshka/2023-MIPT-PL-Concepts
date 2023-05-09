# Tests

## Overview

This directory provides the tests for the `AutoObject` class template's
main features.

The [utils directory](utils) provides tools used for testing the `AutoObject`
class template and the [suits directory](suits) provides the tests themselves.

The tests aim to be samples of the `AutoObject` class template usage and
are considered a part of the documentation.

## Framework

These tests are built by the [building script](../../build.sh) only if
the Google Test framework is installed on the machine.

On macOS the framework can be installed via Homebrew with the following command:

```bash
brew install googletest
```

## Executable

If these tests are built, an executable file named `objects_test` is produced
in this directory. To run the tests one can execute the following command from
this directory:

```bash
./objects_test
```
