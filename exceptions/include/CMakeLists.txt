cmake_minimum_required(VERSION 3.23)
project(except)

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
")

# llvm-ranlib does not have -no_warning_for_no_symbols option
if (NOT CMAKE_RANLIB MATCHES "llvm")
    # silence the "file has no symbols" warning for those .hpp files,
    # for which the respective .cpp file is empty
    set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
endif ()

# enable including files by their relative path to the current directory
# instead of by the relative path to the file where the inclusion happens
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

# build OBJECT sub-libraries
add_subdirectory(utils)
add_subdirectory(objects)
add_subdirectory(types)
add_subdirectory(impl)

# add the except STATIC library target
add_library(except STATIC except)

# link the OBJECT sub-libraries to the except STATIC library
# see https://cmake.org/pipermail/cmake/2018-September/068263.html
# for explanation on combining several libraries into one
target_link_libraries(except impl types objects utils)

# place the resulting library archive in the except/lib directory
# instead of in the build directory produced by cmake
set_target_properties(
        except
        PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../lib"
)
