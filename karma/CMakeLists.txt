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

add_subdirectory(src)

add_executable(karma_play main.cpp)

target_link_libraries(karma_play karma)

target_include_directories(karma_play PUBLIC "${PROJECT_SOURCE_DIR}/src")
