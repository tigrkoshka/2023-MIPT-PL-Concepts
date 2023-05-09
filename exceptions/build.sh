#!/bin/bash

# This script builds the except library and playground

# After the execution the except library archive "libexcept.a" can be found in
# the lib directory, and the playground executable "except_play" can be found
# in the playground directory

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)

# make sure the necessary scripts are executable
chmod +x "${SCRIPT_DIR}/include/build.sh" &&
  chmod +x "${SCRIPT_DIR}/playground/build.sh"

# build the except library and place the archive
# "libexcept.a" in the lib directory
"${SCRIPT_DIR}/include/build.sh" &&

  # build the except playground and place the executable
  # "except_play" in the playground directory
  "${SCRIPT_DIR}/playground/build.sh"
