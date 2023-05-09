enable_testing()
find_package(GTest)

if (${GTest_FOUND})
    enable_testing()

    add_executable(
            objects_test
            suits/00_templates.cpp
            suits/01_constructors.cpp
            suits/02_destructor.cpp
            suits/03_assignment.cpp
            suits/04_conversion.cpp
            suits/05_operators.cpp
            suits/06_methods.cpp
            suits/07_usage.cpp
            suits/fixture.cpp
            utils/sample_class.cpp
            utils/stats.cpp
            utils/concepts.cpp
    )
    target_link_libraries(objects_test ${GTEST_BOTH_LIBRARIES} objects)

    # place the resulting executable file in the current directory
    # instead of in the build directory produced by cmake
    set_target_properties(
            objects_test
            PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    add_test(objects objects_test)
endif ()
