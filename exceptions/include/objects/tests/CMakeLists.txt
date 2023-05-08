enable_testing()
find_package(GTest REQUIRED)

set(
        SOURCES
        suits/00_templates.cpp
        suits/01_constructors.cpp
        suits/02_destructor.cpp
        suits/03_assignments.cpp
        suits/04_conversions.cpp
        suits/05_operators.cpp
        suits/06_methods.cpp
        utils/sample_class.cpp
        utils/stats.cpp
        utils/concepts.cpp
)

add_executable(simple_test ${SOURCES})
target_link_libraries(simple_test GTest::gtest GTest::gtest_main objects)
add_test(simple simple_test)

add_executable(final_test ${SOURCES})
target_link_libraries(final_test GTest::gtest GTest::gtest_main objects)
target_compile_definitions(final_test PRIVATE FINAL)
add_test(final final_test)
