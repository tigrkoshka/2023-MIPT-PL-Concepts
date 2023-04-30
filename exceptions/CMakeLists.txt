# This CMakeLists file is for Clion to build both the except library
# and playground and then run the playground executable

cmake_minimum_required(VERSION 3.23)
project(except_project)

add_subdirectory(include)
add_subdirectory(playground)

add_custom_target(dummy DEPENDS except)
