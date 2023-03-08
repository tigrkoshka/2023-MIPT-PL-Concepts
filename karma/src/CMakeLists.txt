add_subdirectory(errors)
add_subdirectory(specs)
add_subdirectory(utils)

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
        ${errors_srcs}
        ${specs_srcs}
        ${utils_srcs}
        compiler.cpp
        executor.cpp
        disassembler.cpp
)

target_link_libraries(karma errors specs utils)
