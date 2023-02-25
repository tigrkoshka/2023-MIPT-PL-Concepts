cmake_minimum_required(VERSION 3.23)
project(Assembler)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Warith-conversion -Wconversion -Wsign-conversion -Wfloat-conversion")

include_directories(/usr/local/opt/gcc@12/include/c++/12)

add_executable(Assembler main.cpp executor.cpp specs.cpp compiler.cpp compiler.hpp compiler.cpp compiler.hpp compiler.cpp compiler.hpp)
