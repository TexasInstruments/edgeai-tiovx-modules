/*
 *
 * Copyright (c) 2021 Texas Instruments Incorporated
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
#ifndef _TIOVX_MODULES_COMMON
#define _TIOVX_MODULES_COMMON

#include <TI/tivx.h>
#include <TI/tivx_task.h>
#include <TI/tivx_target_kernel.h>
#include <TI/tivx_img_proc.h>
#include <edgeai_tiovx_img_proc.h>

#include <TI/j7_tidl.h>
#include <TI/tivx_fileio.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <float.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define TIOVX_MODULE_DEBUG

#define TIOVX_MODULES_MAX_FILE_PATH_SIZE  (512u)
#define TIOVX_MODULES_MAX_OBJ_NAME_SIZE   (512u)
#define TIOVX_MODULES_MAX_BUFQ_DEPTH      (16u)
#define TIOVX_MODULES_MAX_TENSOR_DIMS     (4u)
#define TIOVX_MODULES_MAX_TENSORS         (8u)
#define TIOVX_MODULES_MAX_PARAMS          (16u)
#define TIOVX_MODULES_MAX_REF_HANDLES     (16u)

#ifdef TIOVX_MODULE_DEBUG
#define TIOVX_MODULE_PRINTF(f_, ...) printf("[DEBUG] %d: %s: "f_, __LINE__, __func__, ##__VA_ARGS__)
#else
#define TIOVX_MODULE_PRINTF(f_, ...)
#endif

#define TIOVX_MODULE_ERROR(f_, ...) printf("[ERROR] %d: %s: "f_, __LINE__, __func__, ##__VA_ARGS__)

typedef enum {
    VISS = 0,
    AEWB,
    LDC,
} EDGEAI_NODES;

typedef struct {
    vx_object_array arr[TIOVX_MODULES_MAX_BUFQ_DEPTH];
    tivx_raw_image image_handle[TIOVX_MODULES_MAX_BUFQ_DEPTH];

    tivx_raw_image_create_params_t params;

    vx_int32 graph_parameter_index;
    vx_int32 bufq_depth;

} RawImgObj;

typedef struct {
    vx_object_array arr[TIOVX_MODULES_MAX_BUFQ_DEPTH];
    vx_image image_handle[TIOVX_MODULES_MAX_BUFQ_DEPTH];

    vx_int32 width;
    vx_int32 height;
    vx_int32 color_format;

    vx_int32 graph_parameter_index;
    vx_int32 bufq_depth;

} ImgObj;

typedef struct {
    vx_object_array arr[TIOVX_MODULES_MAX_BUFQ_DEPTH];
    vx_tensor tensor_handle[TIOVX_MODULES_MAX_BUFQ_DEPTH];

    vx_int32 num_dims;
    vx_int32 dim_sizes[VX_TENSOR_NUMBER_OF_DIMS];
    vx_int32 datatype;

    vx_int32 graph_parameter_index;
    vx_int32 bufq_depth;

} TensorObj;

typedef struct {
    vx_object_array arr[TIOVX_MODULES_MAX_BUFQ_DEPTH];
    vx_pyramid pyramid_handle[TIOVX_MODULES_MAX_BUFQ_DEPTH];

    vx_int32 width;
    vx_int32 height;
    vx_int32 color_format;
    vx_int32 levels;
    vx_float32 scale;

    vx_int32 graph_parameter_index;
    vx_int32 bufq_depth;

} PyramidObj;

typedef struct {
    vx_object_array arr[TIOVX_MODULES_MAX_BUFQ_DEPTH];
    vx_distribution dst_handle[TIOVX_MODULES_MAX_BUFQ_DEPTH];

	vx_size 	num_bins;
    vx_int32 	offset;
    vx_uint32 	range;

    vx_int32 graph_parameter_index;
    vx_int32 bufq_depth;

} DstObj;

#ifdef __cplusplus
}
#endif

#endif //_TIOVX_MODULES_COMMON
