#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# create a karma library archive
cmake -S "${SCRIPT_DIR}" -B "${SCRIPT_DIR}/build" &&
cmake --build "${SCRIPT_DIR}/build" -j 9 &&

# create a lib directory if it does not exist
[ -d "${SCRIPT_DIR}/../lib" ] || mkdir -- "${SCRIPT_DIR}/../lib" &&

# move the archive to the lib directory
cp "${SCRIPT_DIR}/build/libkarma.a" "${SCRIPT_DIR}/../lib/libkarma.a"