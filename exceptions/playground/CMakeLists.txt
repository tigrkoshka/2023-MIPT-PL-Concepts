cmake_minimum_required(VERSION 3.23)
project(except_play)

# require the C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# compilation flags
set(CMAKE_CXX_FLAGS "\
${CMAKE_CXX_FLAGS} \
-Werror \
-Wall \
-Wextra \
-Wconversion \
-Wsign-conversion \
-Wfloat-conversion \
-fsanitize=undefined \
-fno-exceptions \
")

# find the archived library
find_library(EXCEPT_LIB except ${CMAKE_CURRENT_SOURCE_DIR}/../lib)

# find the include path
find_path(EXCEPT_INCLUDES except ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# enable inclusion relative to the include directory
include_directories(${EXCEPT_INCLUDES})

# add the executable target
add_executable(except_play main.cpp)

# link the archived library to the executable target
target_link_libraries(except_play PUBLIC ${EXCEPT_LIB})

# place the resulting executable file in the current directory
# instead of in the build directory produced by cmake
set_target_properties(
        except_play
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
