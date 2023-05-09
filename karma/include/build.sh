#!/bin/bash

# This script builds the karma library

# After the execution the karma library archive "libkarma.a"
# can be found in the karma/lib directory

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)

CMAKE_ARGS=(
  -S "${SCRIPT_DIR}"
  -B "${SCRIPT_DIR}/build"
)

if [[ -n ${COMPILER+x} ]]; then
  CMAKE_ARGS+=(-DCMAKE_CXX_COMPILER="${COMPILER}")
fi

cmake "${CMAKE_ARGS[@]}" &&
  cmake --build "${SCRIPT_DIR}/build" -j 9
