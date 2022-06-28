cmake_minimum_required(VERSION 3.10.0)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/common.cmake)

build_lib(${PROJECT_NAME}  # Named argument: library name
          SHARED           # Named argument: library type
          0.1.0            # Named argument: library version
          # The following all will be clubbed under ${ARGN}
          tiovx_color_convert_module.c
          tiovx_dl_color_convert_module.c
          tiovx_multi_scaler_module.c
          tiovx_img_mosaic_module.c
          tiovx_dl_pre_proc_module.c
          tiovx_dl_color_blend_module.c
          tiovx_sensor_module.c
          tiovx_viss_module.c
          tiovx_ldc_module.c
          tiovx_pyramid_module.c
          tiovx_dof_module.c
          tiovx_dof_viz_module.c
          tiovx_sde_module.c
          tiovx_sde_viz_module.c)

