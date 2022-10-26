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
#ifndef _TIOVX_IMG_MOSAIC_MODULE
#define _TIOVX_IMG_MOSAIC_MODULE

/**
 * \defgroup group_tiovx_img_mosaic_modules Software image mosaic module using MSC
 *
 * \brief This section contains APIs for using the tivxImgMosaicNode
 *
 * \ingroup group_tiovx_modules
 *
 * @{
 */

#include "tiovx_modules_common.h"
#include <TI/tivx_img_proc.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief HW Mosaic Module Data Structure
 *
 * Contains the data objects required to use tivxImgMosaicNode
 *
 */
typedef struct {
    /*! MSC HW Mosaic node object */
    vx_node  node;

    /*! MSC HW Mosaic kernel object */
    vx_kernel kernel;

    /*! MSC HW Mosaic node user data object for configuration of node */
    vx_user_data_object config;

    /*! Mosaic node params structure to initialize config object */
    tivxImgMosaicParams params;

    /*! Color format used by img mosaic node; supported values of \ref VX_DF_IMAGE_U8 and \ref VX_DF_IMAGE_NV12 */
    vx_int32 color_format;

    /*! Array of input image objects  */
    ImgObj inputs[TIVX_IMG_MOSAIC_MAX_INPUTS];

    /*! Total number of input channels */
    vx_int32 num_channels;

    /*! Total number of inputs to mosaic node */
    vx_int32 num_inputs;

    /*! Buffer array of output images of mosaic node */
    vx_image output_image[TIOVX_MODULES_MAX_BUFQ_DEPTH];

    /*! Width of mosaic node output image */
    vx_int32 out_width;

    /*! Height of mosaic node output image */
    vx_int32 out_height;

    /*! Bufq depth of output image */
    vx_int32 out_bufq_depth;

    /*! Flag to write mosaic output to file */
    vx_int32 write_img_mosaic_output;

    /*! Mosaic node graph parameter index of output */
    vx_int32 output_graph_parameter_index;

    /*! Mosaic node graph parameter index of background */
    vx_int32 background_graph_parameter_index;

    /*! Background image applied to output */
    vx_image background_image[TIOVX_MODULES_MAX_BUFQ_DEPTH];

} TIOVXImgMosaicModuleObj;

vx_status tiovx_img_mosaic_module_init(vx_context context, TIOVXImgMosaicModuleObj *obj);
vx_status tiovx_img_mosaic_module_deinit(TIOVXImgMosaicModuleObj *obj);
vx_status tiovx_img_mosaic_module_delete(TIOVXImgMosaicModuleObj *obj);
vx_status tiovx_img_mosaic_module_create(vx_graph graph, TIOVXImgMosaicModuleObj *obj, vx_image background_image, vx_object_array input_arr_user[], const char* target_string);
vx_status tiovx_img_mosaic_module_release_buffers(TIOVXImgMosaicModuleObj *obj);

vx_status tiovx_img_mosaic_module_add_write_output_node(vx_graph graph, TIOVXImgMosaicModuleObj *obj);
vx_status tiovx_img_mosaic_module_send_write_output_cmd(TIOVXImgMosaicModuleObj *obj, vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip);

#ifdef __cplusplus
}
#endif

#endif
