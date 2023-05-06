enable_testing()
find_package(GTest REQUIRED)

add_executable(
        objects_test
        main.cpp
        "${CMAKE_CURRENT_SOURCE_DIR}/../auto.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/../manager.cpp"
)
target_link_libraries(objects_test GTest::gtest GTest::gtest_main)

add_test(Foo objects_test)
