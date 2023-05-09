#!/bin/bash

# This script builds the except library

# After the execution the except library archive "libexcept.a"
# can be found in the except/lib directory

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)

CMAKE_ARGS=(
  -S "${SCRIPT_DIR}"
  -B "${SCRIPT_DIR}/build"
)

if [[ -n ${COMPILER+x} ]]; then
  CMAKE_ARGS+=(-DCMAKE_CXX_COMPILER="${COMPILER}")
fi

cmake "${CMAKE_ARGS[@]}" &&
  cmake --build "${SCRIPT_DIR}/build"
