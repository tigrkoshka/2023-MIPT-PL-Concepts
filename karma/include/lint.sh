#!/bin/bash

# This script runs clang-tidy on the whole karma library
# using CMake and LLVM toolchain (including Clang and STL)
#
# Disclaimer:
# This script works only on MacOS with LLVM@16 installed via Homebrew.
# It was not designed to be portable and was only used during development.

set -e

LLVM="/usr/local/Cellar/llvm/16.0.0"
LLVM_BIN="${LLVM}/bin"

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

cmake \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_COMPILER="${LLVM_BIN}/clang" \
  -DCMAKE_CXX_COMPILER="${LLVM_BIN}/clang++" \
  -DCMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES="${LLVM}/include/c++/v1;${LLVM}/lib/clang/16/include" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
  -S "${SCRIPT_DIR}" \
  -B "${SCRIPT_DIR}/build"

cd "${SCRIPT_DIR}/build"

${LLVM_BIN}/run-clang-tidy \
  -clang-tidy-binary="${LLVM_BIN}/clang-tidy" \
  -config-file="${SCRIPT_DIR}/../.clang-tidy"
