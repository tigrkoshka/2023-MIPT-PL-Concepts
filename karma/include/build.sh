#!/bin/bash

# This script builds the karma library

# After the execution the karma library archive "libkarma.a"
# can be found in the karma/lib directory

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

cmake -S "${SCRIPT_DIR}" -B "${SCRIPT_DIR}/build" &&
cmake --build "${SCRIPT_DIR}/build"
