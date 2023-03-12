cmake_minimum_required(VERSION 3.23)
project(karma)

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
")

# silence the "file has no symbols" warning for those .hpp files,
# for which the respective .cpp file is empty
SET(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
SET(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")

# enable including files by their relative path to the current directory
# instead of by the relative path to the file where the inclusion happens
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

# build OBJECT sub-libraries
add_subdirectory(specs)
add_subdirectory(utils)
add_subdirectory(exec)
add_subdirectory(compiler)
add_subdirectory(executor)
add_subdirectory(disassembler)

# add the karma STATIC library target
add_library(karma STATIC karma)

# link the OBJECT sub-libraries to the karma SOURCE library
# see https://cmake.org/pipermail/cmake/2018-September/068263.html
# for explanation on combining several libraries into one
target_link_libraries(karma compiler executor disassembler exec specs utils)

# place the resulting library archive in the karma/lib directory
# instead of in the build directory produced by cmake
set_target_properties(
        karma
        PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../lib"
)
