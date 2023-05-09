find_package(GTest)

if (${GTest_FOUND})
    enable_testing()

    add_executable(
            except_test
            suits/simple.cpp
            suits/types.cpp
            suits/catch.cpp
            suits/destructors.cpp
            suits/death.cpp
            utils/custom.cpp
            utils/recording.cpp
    )
    target_link_libraries(except_test GTest::gtest GTest::gtest_main except)

    # place the resulting executable file in the current directory
    # instead of in the build directory produced by cmake
    set_target_properties(
            except_test
            PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    add_test(except except_test)
endif ()
