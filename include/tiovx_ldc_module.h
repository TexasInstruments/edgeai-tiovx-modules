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
#ifndef _TIOVX_LDC_MODULE
#define _TIOVX_LDC_MODULE

/**
 * \defgroup group_tiovx_modules_ldc LDC Node Module
 *
 * \brief This section contains module APIs for the TIOVX LDC node tivxVpacLdcNode
 *
 * \ingroup group_tiovx_modules
 *
 * @{
 */

#include "tiovx_modules_common.h"
#include "tiovx_sensor_module.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \brief LDC Mode enumeration
 *
 * Contains different enumeration option for LDC operation
 *
 * 0 - All details are taken from DCC data, no need to provide mesh image,
 *     warp matrix, region params etc. (default)
 * 1 - No DCC data available user to provide all details pertaining to warp matrix,
 *     mesh image, region params etc.
 * 2 - Max enumeration value
 *
 */
typedef enum {
    TIOVX_MODULE_LDC_OP_MODE_DCC_DATA = 0,
    TIOVX_MODULE_LDC_OP_MODE_MESH_IMAGE,
    TIOVX_MODULE_LDC_OP_MODE_MAX,
    TIOVX_MODULE_LDC_OP_MODE_DEFAULT = TIOVX_MODULE_LDC_OP_MODE_DCC_DATA

}eLDCMode;

/** \brief LDC Module Data Structure
 *
 * Contains the data objects required to use tivxVpacLdcNode
 *
 */
typedef struct {
    /*! LDC node object */
    vx_node node;

    /*! LDC node params structure to initialize config object */
    tivx_vpac_ldc_params_t params;

    /*! User data object for config parameter, used as node parameter of LDC node */
    vx_user_data_object config;

    /*! Must be 2x3 (affine) or 3x3 (perspective) */
    vx_matrix warp_matrix;

    /*! LDC mesh params structure to initialize mesh config object */
    tivx_vpac_ldc_mesh_params_t mesh_params;

    /*! User data object for mesh config, used as node parameter of LDC node */
    vx_user_data_object mesh_config;

    /*! LDC region params structure to initialize region params config object */
    tivx_vpac_ldc_region_params_t region_params;

    /*! User data object for region config, used as node parameter of LDC node */
    vx_user_data_object region_config;

    /*! LDC mesh image */
    vx_image mesh_img;

    /*! LDC table width  */
    vx_uint32 table_width;

    /*! LDC table height  */
    vx_uint32 table_height;

    /*! LDC downscale factor  */
    vx_uint32 ds_factor;

    /*! LDC block width */
    vx_uint32 out_block_width;

    /*! LDC block height */
    vx_uint32 out_block_height;

    /*! LDC pixel padding */
    vx_uint32 pixel_pad;

    /*! LDC Output starting x-coordinate */
    vx_uint32 init_x;

    /*! LDC Output starting y-coordinate */
    vx_uint32 init_y;

    /*! User data object for DCC config parameter, used as node parameter of LDC node */
    vx_user_data_object dcc_config;

    /*! DCC config file path for LDC */
    vx_char dcc_config_file_path[TIVX_FILEIO_FILE_PATH_LENGTH];

    /*! LUT file mesh image based LDC config */
    vx_char lut_file_path[TIVX_FILEIO_FILE_PATH_LENGTH];

    /*! LDC input Image Object  */
    ImgObj input;

    /*! LDC output0 Image Object  */
    ImgObj output0;

    /*! LDC output1 Image Object  */
    ImgObj output1;

    /*! Pointer to sensor object */
    SensorObj *sensorObj;

    /*! LDC Operation mode 0-use dcc data, 1-provide mesh image */
    eLDCMode ldc_mode;

    /*! Flag to enable output1  0 - ouptut1 not required, 1 - output1 required */
    vx_int32 en_output1;

    /*! Flag to enable writing LDC output  */
    vx_int32 en_out_image_write;

    /*! Node used to write LDC output */
    vx_node write_node;

    /*! File path used to write LDC node output */
    vx_array file_path;

    /*! File path prefix used to write LDC node output */
    vx_array file_prefix;

    /*! User data object containing write cmd parameters */
    vx_user_data_object write_cmd;

    /*! Output file path for LDC node output */
    vx_char output_file_path[TIVX_FILEIO_FILE_PATH_LENGTH];

}TIOVXLDCModuleObj;

/** \brief LDC module init helper function
 *
 * This LDC init helper function will create all the data objects required to create the LDC
 * node
 *
 * \param [in]  context    OpenVX context which must be created using \ref vxCreateContext
 * \param [out] obj        LDC Module object which gets populated with LDC node data objects
 * \param [in]  sensorObj  Sensor Module object used to initialize LDC data object parameters;
 *                         must be initialized prior to passing to this function
 */
vx_status tiovx_ldc_module_init(vx_context context, TIOVXLDCModuleObj *obj, SensorObj *sensorObj);

/** \brief LDC module deinit helper function
 *
 * This LDC deinit helper function will release all the data objects created during the \ref app_init_ldc call
 *
 * \param [in,out] obj  LDC Module object which contains LDC node data objects which are released in this function
 *
 */
vx_status tiovx_ldc_module_deinit(TIOVXLDCModuleObj *obj);

/** \brief LDC module delete helper function
 *
 * This LDC delete helper function will delete the LDC node and write node that is created during the \ref app_create_graph_ldc call
 *
 * \param [in,out] obj  LDC Module object which contains LDC node objects which are released in this function
 *
 */
vx_status tiovx_ldc_module_delete(TIOVXLDCModuleObj *obj);

/** \brief LDC module create helper function
 *
 * This LDC create helper function will create the node using all the data objects created during the \ref app_init_ldc call.
 * Internally calls \ref app_create_graph_ldc_write_output if en_out_ldc_write is set
 *
 * \param [in]     graph      OpenVX graph that has been created using \ref vxCreateGraph and where the LDC node is created
 * \param [in,out] obj        LDC Module object which contains LDC node and write node which are created in this function
 * \param [in]     input_arr  Input object array to LDC node.  Must be created separately using \ref vxCreateObjectArray
 *
 */
vx_status tiovx_ldc_module_create(vx_graph graph, TIOVXLDCModuleObj *obj, vx_object_array input_arr, const char* target_string);

/** \brief LDC module release buffers helper function
 *
 * This LDC helper function will release the buffers alloted during vxVerifyGraph stage
 *
 * \param [in] obj  LDC Module object
 *
 */
vx_status tiovx_ldc_module_release_buffers(TIOVXLDCModuleObj *obj);

/** \brief LDC module write output helper function
 *
 * This LDC helper function will create the node for writing the LDC output
 *
 * \param [in]     graph   OpenVX graph
 * \param [in,out] obj     LDC Module object which contains LDC node and write node which are created in this function
 *
 */
vx_status tiovx_ldc_module_add_write_output_node(vx_graph graph, TIOVXLDCModuleObj *obj);

/** \brief LDC module write output helper function
 *
 * This LDC helper function will create the node for writing the LDC output
 *
 * \param [in] obj           LDC Module object which contains the write node used in this function
 * \param [in] start_frame   Starting frame to write
 * \param [in] num_frames    Total number of frames to write
 * \param [in] num_skip      Number of capture frames to skip writing
 *
 */
vx_status tiovx_ldc_module_send_write_output_cmd(TIOVXLDCModuleObj *obj, vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip);

/* @} */

#ifdef __cplusplus
}
#endif

#endif
