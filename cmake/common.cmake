include(GNUInstallDirs)

# Specific compile optios across all targets
#add_compile_definitions(MINIMAL_LOGGING)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
ENDIF()

message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE} PROJECT_NAME = ${PROJECT_NAME}")

SET(CMAKE_FIND_LIBRARY_PREFIXES "" "lib")
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".lib" ".so")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})

set(TARGET_PLATFORM     J7)
set(TARGET_CPU          A72)
set(TARGET_OS           LINUX)

add_definitions(
    -DTARGET_CPU=${TARGET_CPU}
    -DTARGET_OS=${TARGET_OS}
)

link_directories(/usr/lib/aarch64-linux-gnu
                 /usr/lib/
                 )

#message("PROJECT_SOURCE_DIR =" ${PROJECT_SOURCE_DIR})
#message("CMAKE_SOURCE_DIR =" ${CMAKE_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/..
                    ${PROJECT_SOURCE_DIR}/../include
                    /usr/local/include
                    /usr/include/processor_sdk/vision_apps/apps/basic_demos/app_rtos/common
                    /usr/include/processor_sdk/vision_apps/apps/basic_demos/app_rtos/rtos_linux
                    /usr/include/processor_sdk/vision_apps/apps/basic_demos/app_rtos/rtos_linux/mpu1
                    /usr/include/processor_sdk/ivision
                    /usr/include/processor_sdk/imaging
                    /usr/include/processor_sdk/tidl_j7/ti_dl/inc
                    /usr/include/processor_sdk/tiovx/include
                    /usr/include/processor_sdk/tiovx/kernels/include
                    /usr/include/processor_sdk/tiovx/kernels_j7/include
                    /usr/include/processor_sdk/tiovx/utils/include
                    /usr/include/processor_sdk/vision_apps
                    /usr/include/processor_sdk/vision_apps/kernels/img_proc/include
                    /usr/include/processor_sdk/vision_apps/kernels/fileio/include
                    )

set(SYSTEM_LINK_LIBS
    tivision_apps
    )

set(COMMON_LINK_LIBS
    edgeai-tiovx-modules
    )

# Function for building a node:
# ARG0: app name
# ARG1: source list
function(build_app)
    set(app ${ARGV0})
    set(src ${ARGV1})
    add_executable(${app} ${${src}})
    target_link_libraries(${app}
                          ${COMMON_LINK_LIBS}
                          ${TARGET_LINK_LIBS}
                          ${SYSTEM_LINK_LIBS}
                         )
endfunction()

