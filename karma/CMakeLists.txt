# This CMakeLists file is for Clion to build both the karma library
# and playground and then run the playground executable

cmake_minimum_required(VERSION 3.23)
project(karma_project)

add_subdirectory(include)
add_subdirectory(playground)

add_custom_target(dummy DEPENDS karma)
