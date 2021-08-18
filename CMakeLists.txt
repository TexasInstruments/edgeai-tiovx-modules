cmake_minimum_required(VERSION 3.0.2)

project(edgeai-tiovx-modules-repo)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

add_subdirectory(src)
add_subdirectory(test)
