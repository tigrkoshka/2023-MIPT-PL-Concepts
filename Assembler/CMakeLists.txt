cmake_minimum_required(VERSION 3.23)
project(Assembler)

set(CMAKE_CXX_STANDARD 20)

include_directories(/usr/local/opt/gcc@12/include/c++/12)

add_executable(Assembler main.cpp executor.cpp executor.hpp specs.hpp utils.hpp)
