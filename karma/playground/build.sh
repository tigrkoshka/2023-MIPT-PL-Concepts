#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

"${SCRIPT_DIR}/../include/build.sh" &&

# create an executable
cmake -S "${SCRIPT_DIR}" -B "${SCRIPT_DIR}/build" &&
cmake --build "${SCRIPT_DIR}/build" -j 9 &&

# move the executable to the root directory
cp "${SCRIPT_DIR}/build/karma_play" "${SCRIPT_DIR}/karma_play"