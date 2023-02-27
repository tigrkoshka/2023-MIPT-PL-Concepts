cmake_minimum_required(VERSION 3.23)
project(Assembler)

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
        Assembler
        main.cpp
        karma/compiler.cpp
        karma/executor.cpp
        karma/disassembler.cpp
        karma/specs/architecture.cpp
        karma/specs/commands.cpp
        karma/specs/exec.cpp
        karma/specs/flags.cpp
        karma/specs/syntax.cpp
        utils/utils.cpp
)
