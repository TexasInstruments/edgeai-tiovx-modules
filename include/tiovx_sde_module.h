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
 #ifndef _TIOVX_SDE_MODULE
 #define _TIOVX_SDE_MODULE

#include "tiovx_modules_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    vx_node node;

    /*! Instance of left input Image object
     *  color formats supported
     *      VX_DF_IMAGE_U8, VX_DF_IMAGE_NV12, VX_DF_IMAGE_U16
     */
    ImgObj input_left;

    /*! Instance of right input Image object
     *  color formats supported
     *      VX_DF_IMAGE_U8, VX_DF_IMAGE_NV12, VX_DF_IMAGE_U16
     */
    ImgObj input_right;

    /*! Instance of output Image object
     *  color format should be VX_DF_IMAGE_S16
     */
    ImgObj output;

    /*! SDE node params structure to initialize config object */
    tivx_dmpac_sde_params_t params;
    /*! User data object for config parameter, used as node parameter of SDE node */
    vx_user_data_object config;

    /* Input parameters */
    vx_int32 num_channels;
    vx_int32 width;
    vx_int32 height;

} TIOVXSdeModuleObj;

/** \brief SDE module init helper function
 *
 * This SDE init helper function will create all the data objects required to create the SDE
 * node
 *
 * \param [in]  context    OpenVX context which must be created using \ref vxCreateContext
 * \param [out] obj        SDE Module object which gets populated with SDE node data objects
 */
vx_status tiovx_sde_module_init(vx_context context, TIOVXSdeModuleObj *obj);

/** \brief SDE module deinit helper function
 *
 * This SDE deinit helper function will release all the data objects created during the \ref app_init_sde call
 *
 * \param [in,out] obj    SDE Module object which contains SDE node data objects which are released in this function
 *
 */
vx_status tiovx_sde_module_deinit(TIOVXSdeModuleObj *obj);

/** \brief SDE module delete helper function
 *
 * This SDE delete helper function will delete the SDE node and write node that is created during the \ref app_create_graph_sde call
 *
 * \param [in,out] obj   SDE Module object which contains SDE node objects which are released in this function
 *
 */
vx_status tiovx_sde_module_delete(TIOVXSdeModuleObj *obj);

/** \brief SDE module create helper function
 *
 * This SDE create helper function will create the node using all the data objects created during the \ref app_init_sde call.
 * Internally calls \ref app_create_graph_sde_write_output if en_out_sde_write is set
 *
 * \param [in]     graph          OpenVX graph that has been created using \ref vxCreateGraph and where the SDE node is created
 * \param [in,out] obj            SDE Module object which contains SDE node and write node which are created in this function
 * \param [in]     input_arr          Pyramid input object array to SDE node.
 * \param [in]     input_ref_arr      Refference Pyramid input object array to SDE node, Typically this will be the delayed version of input_arr.
 * \param [in]     input_flow_vector  (optional) Image input object array for fllow vector input. Required only if enable_temporal_predicton_flow_vector is enabled
 *
 */
vx_status tiovx_sde_module_create(vx_graph graph, TIOVXSdeModuleObj *obj, vx_object_array input_left_arr, vx_object_array input_right_arr, const char* target_string);

/** \brief SDE module release buffers helper function
 *
 * This SDE helper function will release the buffers alloted during vxVerifyGraph stage
 *
 * \param [in] obj  SDE Module object
 *
 */
vx_status tiovx_sde_module_release_buffers(TIOVXSdeModuleObj *obj);

#ifdef __cplusplus
}
#endif

#endif
