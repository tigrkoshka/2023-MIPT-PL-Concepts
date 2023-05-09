#!/bin/bash

# This script builds the karma playground

# This script should be run after the build script in the include directory,
# because building the playground is dependent on building the karma library

# After the execution the karma playground executable "karma_play" can be found
# in the current directory

CMAKE_ARGS=(
  -S "${SCRIPT_DIR}"
  -B "${SCRIPT_DIR}/build"
)

if [[ -n ${COMPILER+x} ]]; then
  CMAKE_ARGS+=(-DCMAKE_CXX_COMPILER="${COMPILER}")
fi

cmake "${CMAKE_ARGS[@]}" &&
  cmake --build "${SCRIPT_DIR}/build"
