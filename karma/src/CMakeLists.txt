add_subdirectory(specs)
add_subdirectory(utils)
add_subdirectory(exec)
add_subdirectory(compiler)
add_subdirectory(executor)
add_subdirectory(disassembler)

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

add_library(
        karma STATIC
        karma.cpp
)

target_link_libraries(karma compiler executor disassembler exec specs utils)
