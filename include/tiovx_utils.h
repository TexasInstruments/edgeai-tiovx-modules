/*
 *
 * Copyright (c) 2017 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef _APP_COMMON
#define _APP_COMMON

#include <TI/tivx.h>
#include <TI/tivx_task.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_kernels_host_utils.h"
#include <TI/tivx_img_proc.h>

#include <TI/j7_tidl.h>
#include <tivx_utils_file_rd_wr.h>
#include <tivx_utils_graph_perf.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <float.h>
#include <math.h>

#include <utils/perf_stats/include/app_perf_stats.h>
#include <utils/console_io/include/app_get.h>

#include "tiovx_modules_common.h"

#ifdef __cplusplus
extern "C" {
#endif

//#define APP_DEBUG
#define APP_USE_FILEIO

#define APP_MAX_FILE_PATH           (256u)
#define APP_ASSERT(x)               assert((x))
#define APP_ASSERT_VALID_REF(ref)   (APP_ASSERT(vxGetStatus((vx_reference)(ref))==VX_SUCCESS));

#define APP_MAX_TENSORS             (4u)
#define APP_MAX_TENSOR_DIMS         (4u)
#define APP_TIDL_MAX_PARAMS         (16u)

#define ABS_FLT(a) ((a) > 0)?(a):(-(a))

#define MAX_IMG_WIDTH  (2048)
#define MAX_IMG_HEIGHT (2048)
#define DISPLAY_WIDTH  (1920)
#define DISPLAY_HEIGHT (1080)
#define NUM_CH    (1)
#define NUM_ALGOS (5)

#define APP_MAX_BUFQ_DEPTH (8)

#ifdef APP_DEBUG
#define APP_PRINTF(f_, ...) printf("[DEBUG] %d: %s: "f_, __LINE__, __func__, ##__VA_ARGS__)
#else
#define APP_PRINTF(f_, ...)
#endif

#define APP_ERROR(f_, ...) printf("[ERROR] %d: %s: "f_, __LINE__, __func__, ##__VA_ARGS__)

#define ALIGN_STRIDE64(width) ((((width) + 32)/64)*64)

vx_status add_graph_parameter_by_node_index(vx_graph graph, vx_node node, vx_uint32 node_parameter_index);

vx_status allocate_single_image_buffer(vx_image image, void *virtAddr[], vx_uint32 sizes[]);
vx_status delete_single_image_buffer(vx_image image, void *virtAddr[], vx_uint32 sizes[]);
vx_status assign_single_image_buffer(vx_image image, void *virtAddr[], vx_uint32 sizes[], vx_uint32 num_planes);
vx_status release_single_image_buffer(vx_image image, void *virtAddr[], vx_uint32 sizes[], vx_uint32 num_planes);

vx_status allocate_image_buffers(ImgObj *imgObj, void *virtAddr[][TIOVX_MODULES_MAX_REF_HANDLES], vx_uint32 sizes[][TIOVX_MODULES_MAX_REF_HANDLES]);
vx_status delete_image_buffers(ImgObj *imgObj, void *virtAddr[][TIOVX_MODULES_MAX_REF_HANDLES], vx_uint32 sizes[][TIOVX_MODULES_MAX_REF_HANDLES]);
vx_status assign_image_buffers(ImgObj *imgObj, void *virtAddr[], vx_uint32 sizes[], vx_int32 bufq);
vx_status release_image_buffers(ImgObj *imgObj, void *virtAddr[], vx_uint32 sizes[], vx_int32 bufq);

vx_status allocate_single_tensor_buffer(vx_tensor tensor, void *virtAddr[], vx_uint32 sizes[]);
vx_status delete_single_tensor_buffer(vx_tensor tensor, void *virtAddr[], vx_uint32 sizes[]);
vx_status assign_single_tensor_buffer(vx_tensor tensor, void *virtAddr[], vx_uint32 sizes[], vx_uint32 num_bufs);
vx_status release_single_tensor_buffer(vx_tensor tensor, void *virtAddr[], vx_uint32 sizes[], vx_uint32 num_bufs);

vx_status allocate_tensor_buffers(TensorObj *tensorObj, void *virtAddr[][TIOVX_MODULES_MAX_REF_HANDLES], vx_uint32 sizes[][TIOVX_MODULES_MAX_REF_HANDLES]);
vx_status delete_tensor_buffers(TensorObj *tensorObj, void *virtAddr[][TIOVX_MODULES_MAX_REF_HANDLES], vx_uint32 sizes[][TIOVX_MODULES_MAX_REF_HANDLES]);
vx_status assign_tensor_buffers(TensorObj *tensorObj, void *virtAddr[], vx_uint32 sizes[], vx_int32 bufq);
vx_status release_tensor_buffers(TensorObj *tensorObj, void *virtAddr[], vx_uint32 sizes[], vx_int32 bufq);

vx_status allocate_single_user_data_buffer(vx_user_data_object user_data, void *virtAddr[], vx_uint32 sizes[]);
vx_status delete_single_user_data_buffer(vx_user_data_object user_data, void *virtAddr[], vx_uint32 sizes[]);
vx_status assign_single_user_data_buffer(vx_user_data_object user_data, void *virtAddr[], vx_uint32 sizes[], vx_uint32 num_bufs);
vx_status release_single_user_data_buffer(vx_user_data_object user_data, void *virtAddr[], vx_uint32 sizes[], vx_uint32 num_bufs);

vx_status allocate_user_data_buffers(vx_object_array obj_arr[], void *virtAddr[][TIOVX_MODULES_MAX_REF_HANDLES], vx_uint32 sizes[][TIOVX_MODULES_MAX_REF_HANDLES], vx_int32 bufq_depth);
vx_status delete_user_data_buffers(vx_object_array obj_arr[], void *virtAddr[][TIOVX_MODULES_MAX_REF_HANDLES], vx_uint32 sizes[][TIOVX_MODULES_MAX_REF_HANDLES],  vx_int32 bufq_depth);
vx_status assign_user_data_buffers(vx_object_array obj_arr[], void *virtAddr[], vx_uint32 sizes[], vx_int32 bufq);
vx_status release_user_data_buffers(vx_object_array obj_arr[], void *virtAddr[], vx_uint32 sizes[], vx_int32 bufq);

vx_status allocate_single_raw_image_buffer(tivx_raw_image image, void *virtAddr[], vx_uint32 sizes[]);
vx_status delete_single_raw_image_buffer(tivx_raw_image image, void *virtAddr[], vx_uint32 sizes[]);
vx_status assign_single_raw_image_buffer(tivx_raw_image image, void *virtAddr[], vx_uint32 sizes[], vx_uint32 num_planes);
vx_status release_single_raw_image_buffer(tivx_raw_image image, void *virtAddr[], vx_uint32 sizes[], vx_uint32 num_planes);

vx_status allocate_raw_image_buffers(RawImgObj *imgObj, void *virtAddr[][TIOVX_MODULES_MAX_REF_HANDLES], vx_uint32 sizes[][TIOVX_MODULES_MAX_REF_HANDLES]);
vx_status delete_raw_image_buffers(RawImgObj *imgObj, void *virtAddr[][TIOVX_MODULES_MAX_REF_HANDLES], vx_uint32 sizes[][TIOVX_MODULES_MAX_REF_HANDLES]);
vx_status assign_raw_image_buffers(RawImgObj *imgObj, void *virtAddr[], vx_uint32 sizes[], vx_int32 bufq);
vx_status release_raw_image_buffers(RawImgObj *imgObj, void *virtAddr[], vx_uint32 sizes[], vx_int32 bufq);

vx_status allocate_single_pyramid_buffer(vx_pyramid pyramid, void *virtAddr[], vx_uint32 sizes[], vx_uint32 *num_planes);
vx_status delete_single_pyramid_buffer(vx_pyramid pyramid, void *virtAddr[], vx_uint32 sizes[], vx_uint32 *num_planes);
vx_status assign_single_pyramid_buffer(vx_pyramid pyramid, void *virtAddr[], vx_uint32 sizes[], vx_uint32 *num_planes);
vx_status release_single_pyramid_buffer(vx_pyramid pyramid, void *virtAddr[], vx_uint32 sizes[], vx_uint32 *num_planes);

vx_status allocate_pyramid_buffers(PyramidObj *pyramidObj, void *virtAddr[][TIOVX_MODULES_MAX_REF_HANDLES], vx_uint32 sizes[][TIOVX_MODULES_MAX_REF_HANDLES]);
vx_status delete_pyramid_buffers(PyramidObj *pyramidObj, void *virtAddr[][TIOVX_MODULES_MAX_REF_HANDLES], vx_uint32 sizes[][TIOVX_MODULES_MAX_REF_HANDLES]);
vx_status assign_pyramid_buffers(PyramidObj *pyramidObj, void *virtAddr[], vx_uint32 sizes[], vx_int32 bufq);
vx_status release_pyramid_buffers(PyramidObj *pyramidObj, void *virtAddr[], vx_uint32 sizes[], vx_int32 bufq);

vx_status readTensor(char* file_name, vx_tensor tensor_o);
vx_status writeTensor(char* file_name, vx_tensor tensor_o);
vx_status create_tensor_mask(vx_tensor tensor_o, vx_int32 num_classes);

vx_status readImage(char* file_name, vx_image img);
vx_status writeImage(char* file_name, vx_image img);
vx_status resetImage(vx_image img, int32_t value);

vx_status readRawImage(char* file_name, tivx_raw_image img);
vx_status writeRawImage(char* file_name, tivx_raw_image img);

#ifdef __cplusplus
}
#endif

#endif
