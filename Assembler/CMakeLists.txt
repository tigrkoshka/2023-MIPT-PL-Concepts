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
-Warith-conversion \
")

include_directories(/usr/local/opt/gcc@12/include/c++/12)

add_executable(
        Assembler
        main.cpp
        compiler.cpp
        executor.cpp
        disassembler.cpp
        utils.cpp
        specs/architecture.cpp
        specs/commands.cpp
        specs/exec.cpp
        specs/flags.cpp
        specs/syntax.cpp
)
