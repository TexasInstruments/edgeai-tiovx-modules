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
 #ifndef _TIOVX_PYRAMID_MODULE
 #define _TIOVX_PYRAMID_MODULE

/**
 * \defgroup group_modules_pyramid Pyramid Node Module
 *
 * \brief This section contains module APIs for the TIOVX Pyramid node tivxVpacMscPyramidNode
 *
 * \ingroup group_modules
 *
 * @{
 */

#include "tiovx_modules_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    vx_node node;

    ImgObj     input;
    PyramidObj output;

    /* Input parameters */
    vx_int32 num_channels;
    vx_int32 width;
    vx_int32 height;

} TIOVXPyramidModuleObj;

/** \brief Pyramid module init helper function
 *
 * This Pyramid init helper function will create all the data objects required to create the Pyramid
 * node
 *
 * \param [in]  context    OpenVX context which must be created using \ref vxCreateContext
 * \param [out] obj        Pyramid Module object which gets populated with Pyramid node data objects
 */
vx_status tiovx_pyramid_module_init(vx_context context, TIOVXPyramidModuleObj *obj);

/** \brief Pyramid module deinit helper function
 *
 * This Pyramid deinit helper function will release all the data objects created during the \ref app_init_pyramid call
 *
 * \param [in,out] obj    Pyramid Module object which contains Pyramid node data objects which are released in this function
 *
 */
vx_status tiovx_pyramid_module_deinit(TIOVXPyramidModuleObj *obj);

/** \brief Pyramid module delete helper function
 *
 * This Pyramid delete helper function will delete the pyramid node and write node that is created during the \ref app_create_graph_pyramid call
 *
 * \param [in,out] obj   Pyramid Module object which contains Pyramid node objects which are released in this function
 *
 */
vx_status tiovx_pyramid_module_delete(TIOVXPyramidModuleObj *obj);

/** \brief Pyramid module create helper function
 *
 * This Pyramid create helper function will create the node using all the data objects created during the \ref app_init_pyramid call.
 *
 * \param [in]     graph          OpenVX graph that has been created using \ref vxCreateGraph and where the Pyramid node is created
 * \param [in,out] obj            Pyramid Module object which contains Pyramid node and write node which are created in this function
 * \param [in]     input_arr      Image input object array to Pyramid node.  Must be created separately, typically passed from output of capture node
 * \param [in]     target_string  Targets on which the Pyramid node should run. Supported values are TIVX_TARGET_VPAC_MSC0, TIVX_TARGET_VPAC_MSC1
 *
 */
vx_status tiovx_pyramid_module_create(vx_graph graph, TIOVXPyramidModuleObj *obj, vx_object_array input_arr, const char* target_string);

/** \brief Pyramid module release buffers helper function
 *
 * This Pyramid helper function will release the buffers alloted during vxVerifyGraph stage
 *
 * \param [in] obj  Pyramid Module object
 *
 */
vx_status tiovx_pyramid_module_release_buffers(TIOVXPyramidModuleObj *obj);

#ifdef __cplusplus
}
#endif

#endif
