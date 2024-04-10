cmake_minimum_required(VERSION 3.23)
project(karma_play)

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
")

# GCC has problems with the address sanitizer and the undefined sanitizer in MacOS with Apple Silicon
if (NOT CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined")
endif ()

# find the archived library
find_library(KARMA_LIB karma ${CMAKE_CURRENT_SOURCE_DIR}/../lib)

# find the include path
find_path(KARMA_INCLUDES karma ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# enable inclusion relative to the include directory
include_directories(${KARMA_INCLUDES})

# add the executable target
add_executable(karma_play main.cpp)

# link the archived library to the executable target
target_link_libraries(karma_play PUBLIC ${KARMA_LIB})

# place the resulting executable file in the current directory
# instead of in the build directory produced by cmake
set_target_properties(
        karma_play
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
