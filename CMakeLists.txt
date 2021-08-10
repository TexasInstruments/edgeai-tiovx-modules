cmake_minimum_required(VERSION 3.0.2)

project(edgeai-tiovx-modules-repo)

exec_program(pkg-config ARGS --cflags glib-2.0 OUTPUT_VARIABLE GLIB_FLAGS)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GLIB_FLAGS}")

add_subdirectory(src)
add_subdirectory(test)
