enable_testing()
find_package(GTest REQUIRED)

add_executable(
        except_test
        suits/01_simple.cpp
        suits/02_types.cpp
        suits/03_catch.cpp
        suits/04_destructors.cpp
        utils/custom.cpp
        utils/recording.cpp
        suits/05_death.cpp)
target_link_libraries(except_test GTest::gtest GTest::gtest_main except)
add_test(except except_test)
