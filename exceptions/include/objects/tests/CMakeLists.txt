enable_testing()
find_package(GTest REQUIRED)

add_executable(
        objects_test
        suits/00_templates.cpp
        suits/01_constructors.cpp
        suits/02_destructor.cpp
        suits/03_assignment.cpp
        suits/04_conversion.cpp
        suits/05_operators.cpp
        suits/06_methods.cpp
        suits/fixture.cpp
        utils/sample_class.cpp
        utils/stats.cpp
        utils/concepts.cpp
)
target_link_libraries(objects_test GTest::gtest GTest::gtest_main objects)
add_test(objects objects_test)
