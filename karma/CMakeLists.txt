# This CMakeLists file is purely for Clion to build both the karma library
# and playground by executing the build.sh script

cmake_minimum_required(VERSION 3.23)
project(karma_project)

# these lines are purely a workaround for Clion to see
# the karma and karma_play targets
add_subdirectory(include)
add_subdirectory(playground)

# execute the build.sh file on the cmake build stage
add_custom_target(dummy ALL "${CMAKE_CURRENT_SOURCE_DIR}/build.sh")
