cmake_minimum_required(VERSION 3.23)
project(karma_play)

# require the C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# compilation flags
set(CMAKE_CXX_FLAGS "\
${CMAKE_CXX_FLAGS} \
-Wall \
-Wextra \
-Wconversion \
-Wsign-conversion \
-Wfloat-conversion \
-fsanitize=undefined \
")

# find the archived library
find_library(KARMA_LIB karma lib)

# find the include path
find_path (KARMA_INCLUDES karma.hpp include)

# enable inclusion relative to the include directory
include_directories(${KARMA_INCLUDES})

# add the executable target
add_executable(karma_play main.cpp)

# link the archived library to the executable target
target_link_libraries(karma_play PUBLIC ${KARMA_LIB})
