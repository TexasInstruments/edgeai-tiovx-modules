cmake_minimum_required(VERSION 3.10.0)

set(PROJ_NAME edgeai-tiovx-modules-test)

project(${PROJ_NAME})

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/common.cmake)

set(EDGEAI_TIOVX_MODULES_TEST
    app_tiovx_color_convert_module_test.c
    app_tiovx_multi_scaler_module_test.c
    app_tiovx_img_mosaic_module_test.c
    app_tiovx_dl_pre_proc_module_test.c
    app_tiovx_dl_color_blend_module_test.c
    app_tiovx_dl_draw_box_module_test.c
    app_common.c
    main.c)

build_app(${PROJ_NAME} EDGEAI_TIOVX_MODULES_TEST)

