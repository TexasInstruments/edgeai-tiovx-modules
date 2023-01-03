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
 #ifndef _TIOVX_MULTI_SCALER_MODULE
 #define _TIOVX_MULTI_SCALER_MODULE

/**
 * \defgroup group_tiovx_multi_scaler_module Multi-scaler OpenVx module
 *
 * \brief This section contains host side module APIs for using TIOVX node tivxVpacMscScaleNode
 *
 * \ingroup group_tiovx_modules
 *
 * @{
 */

#include "tiovx_modules_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Maximum amount of values allowed from scaler node
 *
 */
#define TIOVX_MULTI_SCALER_MODULE_MAX_OUTPUTS (5)

/** \brief TIOVX multi-scaler module object
 *
 * Contains the data objects required to use tivxVpacMscScaleNode
 *
 */
typedef struct {
    /*! Node object */
    vx_node node;

    /*! Input image object */
    ImgObj input;

    /*! Array of output image objects */
    ImgObj output[TIOVX_MULTI_SCALER_MODULE_MAX_OUTPUTS];

    /*! Filter coefficients data object */
    vx_user_data_object coeff_obj;

    /*! Crop params data objects */
    vx_user_data_object crop_obj[TIOVX_MULTI_SCALER_MODULE_MAX_OUTPUTS];

    /*! Crop params */
    tivx_vpac_msc_crop_params_t crop_params[TIOVX_MULTI_SCALER_MODULE_MAX_OUTPUTS];

    /*! Number of channels to process in a batch */
    vx_int32 num_channels;

    /*! Number of outputs for a given inputl; maximum supported value is \ref TIOVX_MULTI_SCALER_MODULE_MAX_OUTPUTS */
    vx_int32 num_outputs;

    /*! Color format used by scaler node; supported values are \ref VX_DF_IMAGE_U8 and \ref VX_DF_IMAGE_NV12 */
    vx_int32 color_format;

    /*! Interpolation methods; supported values are \ref VX_INTERPOLATION_BILINEAR and \ref VX_INTERPOLATION_NEAREST_NEIGHBOR */
    vx_int32 interpolation_method;

    /*! Flag to enable writing output  */
    vx_int32 en_multi_scalar_output;

    /*! File path used to write output */
    vx_array file_path;

    /*! File path prefix used to write output */
    vx_array file_prefix[TIOVX_MULTI_SCALER_MODULE_MAX_OUTPUTS];

    /*! Write node to save output */
    vx_node write_node[TIOVX_MULTI_SCALER_MODULE_MAX_OUTPUTS];

    /*! Control commands to send to write node */
    vx_user_data_object write_cmd[TIOVX_MULTI_SCALER_MODULE_MAX_OUTPUTS];

    /*! Output file path to write output */
    vx_char output_file_path[TIVX_FILEIO_FILE_PATH_LENGTH];

} TIOVXMultiScalerModuleObj;

/** \brief TIOVX Multi-scaler module init function
 *
 * This function will initialize all the data objects required to create tivxVpacMscScaleNode
 *
 * \param [in]  context OpenVX context which must be created using \ref vxCreateContext
 * \param [out] obj Handle to TIOVX Multi-scaler module object
 *
 */
vx_status tiovx_multi_scaler_module_init(vx_context context, TIOVXMultiScalerModuleObj *obj);

/** \brief TIOVX Multi-scaler module deinit function
 *
 * This function will de-initialize all the data objects created during module init
 *
 * \param [in, out] obj Handle to TIOVX Multi-scaler module object
 *
 */
vx_status tiovx_multi_scaler_module_deinit(TIOVXMultiScalerModuleObj *obj);

/** \brief TIOVX Multi-scaler module delete function
 *
 * This function will delete the multi-scaler node and write node that is created during module create
 *
 * \param [in,out] obj Handle to TIOVX Multi-scaler module object
 *
 */
vx_status tiovx_multi_scaler_module_delete(TIOVXMultiScalerModuleObj *obj);

/** \brief TIOVX Multi-scaler module create function
 *
 * This function will create the node using all the data objects created during module init
 * Internally calls \ref app_create_graph_scaler_write_output if en_multi_scalar_output is set
 *
 * \param [in]     graph      OpenVX graph that has been created using \ref vxCreateGraph and where the scaler node is created
 * \param [in,out] obj        Handle to TIOVX multi scaler module object
 * \param [in]     input_arr   Input object array to Scaler node.  Must be created separately using \ref vxCreateObjectArray
 * \param [in]     target_string   Targets on which the Scaler node should run. Supported values are TIVX_TARGET_VPAC_MSC0, TIVX_TARGET_VPAC_MSC1
 *
 */
vx_status tiovx_multi_scaler_module_create(vx_graph graph, TIOVXMultiScalerModuleObj *obj, vx_object_array input_arr, const char* target_string);

/** \brief TIOVX Multi-scaler module release buffers function
 *
 * This function will help release the data buffers created for multi-scaler OpenVx objects called during
 * vxVerifyGraph stage. This function can be called by the user if an externally allocated buffer is to be
 * provided for processing. The externally allocated buffer should be allocated using the SAME APIs and buffer
 * pools used by TIOVX
 *
 * \param [in,out] obj  Handle to TIOVX multi scaler module object
 *
 */
vx_status tiovx_multi_scaler_module_release_buffers(TIOVXMultiScalerModuleObj *obj);

/** \brief TIOVX Multi-scaler module add write node function
 *
 * This is a helper function used to add a write node to the graph if en_multi_scalar_output is set.
 *
 * \param [in]     graph       OpenVX graph that has been created using \ref vxCreateGraph and where the scaler node is created
 * \param [in,out] obj         Handle to TIOVX multi scaler module object
 * \param [in]     output_idx  Output index of scaler images to write [0-4]
 *
 */
vx_status tiovx_multi_scaler_module_add_write_output_node(vx_graph graph, TIOVXMultiScalerModuleObj *obj, vx_int32 output_idx);

/** \brief TIOVX Multi-scaler module send write output command function
 *
 * This is a helper function used to send commands to write output to a file.
 *
 * \param [in] obj           Handle to TIOVX multi scaler module object
 * \param [in] start_frame   Starting frame to write
 * \param [in] num_frames    Total number of frames to write
 * \param [in] num_skip      Number of frames to skip while writing
 *
 */
vx_status tiovx_multi_scaler_module_send_write_output_cmd(TIOVXMultiScalerModuleObj *obj, vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip);

/** \brief TIOVX Multi-scaler module helper function for setting MSC coefficients
 *
 * Filter coefficients are set based on the type of \ref interpolation_method
 *
 * \param [out] coeff Multi-scaler coefficients as per \ref tivx_vpac_msc_coefficients_t
 * \param [in]  interpolation_method  interpolation type; valid values are \ref VX_INTERPOLATION_BILINEAR and \ref VX_INTERPOLATION_NEAREST_NEIGHBOR
 *
 */
void tiovx_multi_scaler_module_set_coeff(tivx_vpac_msc_coefficients_t *coeff,  uint32_t interpolation_method);

/** \brief TIOVX Multi-scaler module update filter coefficients function
 *
 * This function will help send a contorl command to the remote target to update the filter coefficients
  *
 * \param [in]     graph          OpenVX graph that has been created using \ref vxCreateGraph and where the scaler node is created
 * \param [in,out] obj            Handle to TIOVX multi scaler module object
 * \param [in]     input_arr      Input object array to Scaler node.  Must be created separately using \ref vxCreateObjectArray
 * \param [in]     target_string  Targets on which the Scaler node should run. Supported values are TIVX_TARGET_VPAC_MSC0, TIVX_TARGET_VPAC_MSC1
 *
 */
vx_status tiovx_multi_scaler_module_update_filter_coeffs(TIOVXMultiScalerModuleObj *obj);

/** \brief TIOVX Multi-scaler module initialize crop params
 *
 * This function will initialize the crop params
 *
 * \param [out] obj Handle to TIOVX Multi-scaler module object
 *
 */
void tiovx_multi_scaler_module_crop_params_init(TIOVXMultiScalerModuleObj *obj);

/** \brief TIOVX Multi-scaler module update crop params
 *
 * This function will set the crop params
 *
 * \param [out] obj Handle to TIOVX Multi-scaler module object
 *
 */
vx_status tiovx_multi_scaler_module_update_crop_params(TIOVXMultiScalerModuleObj *obj);

/* @} */

#ifdef __cplusplus
}
#endif

#endif
