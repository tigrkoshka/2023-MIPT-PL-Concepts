cmake_minimum_required(VERSION 3.23)
project(karma)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "\
${CMAKE_CXX_FLAGS} \
-Wall \
-Wextra \
-Wconversion \
-Wsign-conversion \
-Wfloat-conversion \
-fsanitize=undefined \
")

add_executable(
        karma
        main.cpp
        src/compiler.cpp
        src/executor.cpp
        src/disassembler.cpp
        src/errors/compiler_errors.cpp
        src/errors/disassembler_errors.cpp
        src/errors/executor_errors.cpp
        src/errors/error.cpp
        src/specs/architecture.cpp
        src/specs/commands.cpp
        src/specs/exec.cpp
        src/specs/flags.cpp
        src/specs/syntax.cpp
        src/utils/utils.cpp
)
