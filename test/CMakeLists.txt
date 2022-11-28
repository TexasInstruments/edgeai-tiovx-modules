cmake_minimum_required(VERSION 3.10.0)

set(PROJ_NAME edgeai-tiovx-modules-test)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/common.cmake)

set(SRC_FILES
    app_tiovx_multi_scaler_module_test.c
    app_tiovx_img_mosaic_module_test.c
    app_tiovx_viss_module_test.c
    app_tiovx_ldc_module_test.c
    app_tiovx_pyramid_module_test.c
    app_tiovx_dl_color_convert_module_test.c
    app_tiovx_dl_pre_proc_module_test.c
    app_tiovx_dl_color_blend_module_test.c
    main.c)

if ("${TARGET_SOC}" STREQUAL "J721E" OR "${TARGET_SOC}" STREQUAL "J721S2" OR "${TARGET_SOC}" STREQUAL "J784S4")
    list(APPEND
         SRC_FILES
         app_tiovx_color_convert_module_test.c
         app_tiovx_dof_module_test.c
         app_tiovx_dof_viz_module_test.c
         app_tiovx_sde_module_test.c
         app_tiovx_sde_viz_module_test.c)
endif()

build_app(${PROJ_NAME}
          ${SRC_FILES})
