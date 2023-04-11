cmake_minimum_required(VERSION 3.10.0)

project(edgeai-tiovx-modules)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/common.cmake)

set(SRC_FILES
    src/tiovx_multi_scaler_module.c
    src/tiovx_img_mosaic_module.c
    src/tiovx_sensor_module.c
    src/tiovx_viss_module.c
    src/tiovx_ldc_module.c
    src/tiovx_pyramid_module.c
    src/tiovx_dl_color_convert_module.c
    src/tiovx_dl_pre_proc_module.c
    src/tiovx_dl_color_blend_module.c
    src/tiovx_utils.c)

if ("${TARGET_SOC}" STREQUAL "J721E" OR "${TARGET_SOC}" STREQUAL "J721S2" OR "${TARGET_SOC}" STREQUAL "J784S4")
    list(APPEND
         SRC_FILES
         src/tiovx_color_convert_module.c
         src/tiovx_dof_module.c
         src/tiovx_dof_viz_module.c
         src/tiovx_sde_module.c
         src/tiovx_sde_viz_module.c)
endif()

build_lib(${PROJECT_NAME}  # Named argument: library name
          SHARED           # Named argument: library type
          0.1.0            # Named argument: library version
          # The following all will be clubbed under ${ARGN}
          ${SRC_FILES})

add_subdirectory(test)

option(INSTALL_SRC      "Install src under /opt" OFF)
option(ENABLE_DCC_TOOL  "Enable DCC Tuning Tool Support" OFF)

if (INSTALL_SRC)
    install(DIRECTORY   ${CMAKE_CURRENT_SOURCE_DIR}/
            DESTINATION /opt/${PROJECT_NAME}
            PATTERN     "*/build*" EXCLUDE
            PATTERN     "*/lib*"   EXCLUDE
            PATTERN     "*/bin*"   EXCLUDE
            PATTERN     "*/data*"  EXCLUDE
            PATTERN     "*/.git*"  EXCLUDE)
endif()

if(ENABLE_DCC_TOOL)
add_definitions(-DENABLE_DCC_TOOL)
endif()
