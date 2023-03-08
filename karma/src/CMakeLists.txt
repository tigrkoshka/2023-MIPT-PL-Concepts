add_subdirectory(errors)
add_subdirectory(specs)
add_subdirectory(utils)

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
