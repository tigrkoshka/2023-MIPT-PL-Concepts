#!/bin/bash

# This script builds the karma library and playground

# After the execution the karma library archive "libkarma.a" can be found in
# the lib directory, and the playground executable "karma_play" can be found
# in the playground directory

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)

# make sure the necessary scripts are executable
chmod +x "${SCRIPT_DIR}/include/build.sh" &&
  chmod +x "${SCRIPT_DIR}/playground/build.sh" &&

  # build the karma library and place the archive
  # "libkarma.a" in the lib directory
  "${SCRIPT_DIR}/include/build.sh" &&

  # build the karma playground and place the executable
  # "karma_play" in the playground directory
  "${SCRIPT_DIR}/playground/build.sh"
