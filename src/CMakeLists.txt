cmake_minimum_required(VERSION 3.10.0)

project(edgeai-tiovx-modules)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/common.cmake)

add_library(${PROJECT_NAME} SHARED
            tiovx_color_convert_module.c
            tiovx_multi_scaler_module.c
            tiovx_img_mosaic_module.c
            tiovx_dl_pre_proc_module.c
            tiovx_dl_color_blend_module.c
            tiovx_sensor_module.c
            tiovx_viss_module.c
            tiovx_ldc_module.c
            tiovx_pyramid_module.c)

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/lib/${PROJECT_NAME})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../include DESTINATION /usr/include/${PROJECT_NAME} FILES_MATCHING PATTERN "*.h")

SET_TARGET_PROPERTIES(${PROJECT_NAME}
                        PROPERTIES
                            SOVERSION 0.1.0
                    )
