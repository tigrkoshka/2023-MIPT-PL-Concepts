#!/bin/bash

# This script builds the except playground

# This script should be run after the build script in the include directory,
# because building the playground is dependent on building the except library

# After the execution the except playground executable "except_play" can be found
# in the current directory

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
