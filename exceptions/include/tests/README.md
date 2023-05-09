# Tests

## Overview

This directory provides the tests for the *except* library's main features.

The [utils](utils) directory provides tools used for testing the *except*
library and the [suits](suits) directory provides the tests themselves.

The tests aim to be samples of the *except* library usage and are considered
a part of the documentation.

All the symbols introduced by this directory as well as all the tests are placed
inside the `except::test::impl` namespace.

## Framework

These tests are built by the [building script](../../build.sh) only if
the Google Test framework is installed on the machine.

On macOS the framework can be installed via Homebrew with the following command:

```bash
brew install googletest
```

## Executable

If these tests are built, an executable file named `except_test` is produced
in this directory. To run the tests one can execute the following command from
this directory:

```bash
./except_test
```
