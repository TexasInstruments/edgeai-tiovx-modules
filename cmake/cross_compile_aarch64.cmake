# Usage:
# source ../cmake/setup_cross_compile.sh
# cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/cross_compile_aarch64.cmake ..

set(CMAKE_SYSTEM_NAME      Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

if (NOT DEFINED ENV{CROSS_COMPILER_PATH})
    message(FATAL_ERROR "CROSS_COMPILER_PATH not defined.")
endif()

if (NOT DEFINED ENV{CROSS_COMPILER_PREFIX})
    message(FATAL_ERROR "CROSS_COMPILER_PREFIX not defined.")
endif()

if (NOT DEFINED ENV{TARGET_FS})
    message(FATAL_ERROR "TARGET_FS not defined.")
endif()

set(CROSS_COMPILER_PATH     $ENV{CROSS_COMPILER_PATH})
set(CROSS_COMPILER_PREFIX   $ENV{CROSS_COMPILER_PREFIX})
set(TARGET_FS               $ENV{TARGET_FS})

set(CMAKE_SYSROOT           ${TARGET_FS})
set(CROSS_COMPILER_PREFIX   aarch64-none-linux-gnu-)
set(TOOLCHAIN_PREFIX        ${CROSS_COMPILER_PATH}/bin/${CROSS_COMPILER_PREFIX})
set(CMAKE_C_COMPILER        ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER      ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_AR                ${TOOLCHAIN_PREFIX}ar)
set(CMAKE_LINKER            ${TOOLCHAIN_PREFIX}ld)
set(CMAKE_OBJCOPY           ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_RANLIB            ${TOOLCHAIN_PREFIX}ranlib)
set(CMAKE_SIZE              ${TOOLCHAIN_PREFIX}size)
set(CMAKE_STRIP             ${TOOLCHAIN_PREFIX}strip)

# search programs, headers and, libraries in the target environment
# programs: make etc
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Skip the test program compilation
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# Use list(APPEND) rather than set() so that any variables added by CMake aren't lost!
#
# Here is the docs for this variable: 
# https://cmake.org/cmake/help/latest/variable/CMAKE_TRY_COMPILE_PLATFORM_VARIABLES.html
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES CROSS_COMPILER_PATH TARGET_FS TOOLCHAIN_PREFIX)

link_directories(${TARGET_FS}/usr/lib/aarch64-linux
                 ${TARGET_FS}/usr/lib/python3.8/site-packages/dlr
                 ${TARGET_FS}/usr/lib
                 ${TARGET_FS}/lib
                 )

include_directories(${PROJECT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/..
                    ${PROJECT_SOURCE_DIR}/../include
                    ${TARGET_FS}/usr/local/include
                    ${TARGET_FS}/usr/include/processor_sdk/vision_apps/platform/${TARGET_SOC_LOWER}/rtos/common
                    ${TARGET_FS}/usr/include/processor_sdk/vision_apps/platform/${TARGET_SOC_LOWER}/rtos/common_linux
                    ${TARGET_FS}/usr/include/processor_sdk/vision_apps/platform/${TARGET_SOC_LOWER}/linux
                    ${TARGET_FS}/usr/include/processor_sdk/ivision
                    ${TARGET_FS}/usr/include/processor_sdk/imaging
                    ${TARGET_FS}/usr/include/processor_sdk/perception/include
                    ${TARGET_FS}/usr/include/processor_sdk/imaging/algos/ae/include
                    ${TARGET_FS}/usr/include/processor_sdk/imaging/algos/awb/include
                    ${TARGET_FS}/usr/include/processor_sdk/imaging/algos/dcc/include
                    ${TARGET_FS}/usr/include/processor_sdk/imaging/sensor_drv/include
                    ${TARGET_FS}/usr/include/processor_sdk/imaging/ti_2a_wrapper/include
                    ${TARGET_FS}/usr/include/processor_sdk/imaging/kernels/include
                    ${TARGET_FS}/usr/include/processor_sdk/tidl_j7/ti_dl/inc
                    ${TARGET_FS}/usr/include/processor_sdk/tiovx/include
                    ${TARGET_FS}/usr/include/processor_sdk/tiovx/kernels/include
                    ${TARGET_FS}/usr/include/processor_sdk/tiovx/kernels_j7/include
                    ${TARGET_FS}/usr/include/processor_sdk/tiovx/utils/include
                    ${TARGET_FS}/usr/include/processor_sdk/vision_apps
                    ${TARGET_FS}/usr/include/processor_sdk/vision_apps/utils/app_init/include
                    ${TARGET_FS}/usr/include/processor_sdk/vision_apps/kernels/img_proc/include
                    ${TARGET_FS}/usr/include/processor_sdk/vision_apps/kernels/fileio/include
                    ${TARGET_FS}/usr/include/processor_sdk/vision_apps/kernels/stereo/include
                   )

set(TARGET_LINK_LIBS
    dlr)

#message(STATUS "CMAKE_FIND_ROOT_PATH: ${CMAKE_FIND_ROOT_PATH}")
