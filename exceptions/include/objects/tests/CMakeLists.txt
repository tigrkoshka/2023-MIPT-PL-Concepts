enable_testing()
find_package(GTest)

if (${GTest_FOUND})
    enable_testing()

    add_executable(
            objects_test
            suits/templates.cpp
            suits/constructors.cpp
            suits/destructor.cpp
            suits/assignment.cpp
            suits/conversion.cpp
            suits/operators.cpp
            suits/methods.cpp
            suits/usage.cpp
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
