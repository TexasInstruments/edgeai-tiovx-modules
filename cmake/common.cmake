include(GNUInstallDirs)

add_compile_options(-Wall)

# Specific compile optios across all targets
#add_compile_definitions(MINIMAL_LOGGING)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
ENDIF()

message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE} PROJECT_NAME = ${PROJECT_NAME}")

SET(CMAKE_FIND_LIBRARY_PREFIXES "" "lib")
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".lib" ".so")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_INSTALL_LIBDIR           lib)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr CACHE PATH "Installation Prefix" FORCE)
endif()

set(TARGET_PLATFORM     AM62)
set(TARGET_CPU          A53)
set(TARGET_OS           LINUX)
set(TARGET_SOC          AM62A)
set(TARGET_SOC_LOWER    am62a)

add_definitions(
    -DTARGET_CPU=${TARGET_CPU}
    -DTARGET_OS=${TARGET_OS}
    -DSOC_${TARGET_SOC}
)

link_directories(/usr/lib/aarch64-linux-gnu
                 /usr/lib/
                 )

#message("PROJECT_SOURCE_DIR = ${PROJECT_SOURCE_DIR}")
#message("CMAKE_SOURCE_DIR   = ${CMAKE_SOURCE_DIR}")

include_directories(${PROJECT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/include
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

set(SYSTEM_LINK_LIBS
    tivision_apps
    )

set(COMMON_LINK_LIBS
    edgeai-tiovx-modules
    )

# Function for building a node:
# app_name: app name
# ${ARGN} expands everything after the last named argument to the end
# usage: build_app(app_name a.c b.c....)
function(build_app app_name)
    add_executable(${app_name} ${ARGN})
    target_link_libraries(${app_name}
                          ${COMMON_LINK_LIBS}
                          ${TARGET_LINK_LIBS}
                          ${SYSTEM_LINK_LIBS}
                         )

    set(BIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
    FILE(GLOB BINS ${CMAKE_CURRENT_SOURCE_DIR}/../bin/${CMAKE_BUILD_TYPE}/*)

    install(FILES ${BINS} DESTINATION ${BIN_INSTALL_DIR})
endfunction()

# Function for building a node:
# lib_name: Name of the library
# lib_type: (STATIC, SHARED)
# lib_ver: Version string of the library
# ${ARGN} expands everything after the last named argument to the end
# usage: build_lib(lib_name lib_type lib_ver a.c b.c ....)
function(build_lib lib_name lib_type lib_ver)
    add_library(${lib_name} ${lib_type} ${ARGN})

    SET_TARGET_PROPERTIES(${lib_name}
                          PROPERTIES
                          VERSION ${lib_ver}
                         )

    set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

    install(TARGETS ${lib_name}
            EXPORT ${lib_name}Targets
            LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}  # Shared Libs
            ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}  # Static Libs
            RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}  # Executables, DLLs
           )

    FILE(GLOB HDRS ${CMAKE_CURRENT_SOURCE_DIR}/../include/*.h)

    # Specify the header files to install
    install(FILES ${HDRS} DESTINATION ${INCLUDE_INSTALL_DIR})

endfunction()

