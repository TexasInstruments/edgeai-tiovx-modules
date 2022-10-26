/*
 *
 * Copyright (c) 2020 Texas Instruments Incorporated
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
#ifndef _TIOVX_VISS_MODULE
#define _TIOVX_VISS_MODULE

/**
 * \defgroup group_modules_viss VISS Node Module
 *
 * \brief This section contains module APIs for the TIOVX VISS node tivxVpacVissNode
 *
 * \ingroup group_modules
 *
 * @{
 */

#include "tiovx_modules_common.h"
#include "tiovx_sensor_module.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TIOVX_VISS_MODULE_OUTPUT_NA (0)
#define TIOVX_VISS_MODULE_OUTPUT_EN (1)

/** \brief VISS Module Data Structure
 *
 * Contains the data objects required to use tivxVpacVissNode
 *
 */
typedef struct {
    /*! VISS node object */
    vx_node node;

    /*! User data object for config parameter, used as node parameter of VISS node */
    vx_user_data_object config;

    /*! VISS node params structure to initialize config object */
    tivx_vpac_viss_params_t params;

    /*! User data object for DCC parameter, used as node parameter of VISS node */
    vx_user_data_object dcc_config;

    /*! DCC config file path for VISS */
    vx_char dcc_config_file_path[TIVX_FILEIO_FILE_PATH_LENGTH];

    /*! Object array of AE-AWB result from 2A algorithm, used as node parameter of VISS node */
    vx_object_array ae_awb_result_arr[TIOVX_MODULES_MAX_BUFQ_DEPTH];

    /*! First reference of AE-AWB result from ae_awb_result_arr */
    vx_user_data_object ae_awb_result_handle[TIOVX_MODULES_MAX_BUFQ_DEPTH];

    /* graph_parameter index of ae_awb_result */
    vx_int32 ae_awb_result_graph_parameter_index;

    /* Buf pool dept of ae_awb_result */
    vx_int32 ae_awb_result_bufq_depth;

    /*! Object array of H3A objects, used as node parameter of VISS node */
    vx_object_array h3a_stats_arr[TIOVX_MODULES_MAX_BUFQ_DEPTH];

    /*! First reference of h3a status from h3a_stats_arr */
    vx_user_data_object h3a_stats_handle[TIOVX_MODULES_MAX_BUFQ_DEPTH];

    /* graph_parameter index of h3a_stats */
    vx_int32 h3a_stats_graph_parameter_index;

    /* Buf pool dept of h3a_stats output from VISS */
    vx_int32 h3a_stats_bufq_depth;

    /*! Instance of Raw image object  */
    RawImgObj input;

    /*! Instance of output0 object  */
    ImgObj output0;
    /*! Instance of output1 object  */
    ImgObj output1;
    /*! Instance of output2 object  */
    ImgObj output2;
    /*! Instance of output3 object  */
    ImgObj output3;
    /*! Instance of output4 object  */
    ImgObj output4;

    /*! Output selector mapped to 5 mux outputs.
     * If value is 0 then output is not required -> TIOVX_VISS_MODULE_OUTPUT_NA
     * If value is 1 then output is required -> TIOVX_VISS_MODULE_OUTPUT_EN
     *    - Type of output indicated by tivx_vpac_viss_params_t
     */
    vx_int32 output_select[5];

    /*! Pointer to sensor object */
    SensorObj *sensorObj;

    /*! Flag to indicate whether or not the intermediate output is written */
    vx_int32 en_out_viss_write;

    /*! Node used to write VISS output */
    vx_node img_write_node;

    /*! Node used to write H3A output */
    vx_node h3a_write_node;

    /*! File path used to write VISS node output */
    vx_array file_path;

    /*! File path prefix used to write VISS output */
    vx_array img_file_prefix;

    /*! File path prefix used to write H3A output */
    vx_array h3a_file_prefix;

    /*! User data object containing write cmd parameters */
    vx_user_data_object write_cmd;

    /*! Output file path for VISS node output */
    vx_char output_file_path[TIOVX_MODULES_MAX_OBJ_NAME_SIZE];

    /*! Name of VISS module */
    vx_char obj_name[TIOVX_MODULES_MAX_OBJ_NAME_SIZE];

}TIOVXVISSModuleObj;

/** \brief VISS module init helper function
 *
 * This VISS init helper function will create all the data objects required to create the VISS
 * node
 *
 * \param [in]  context    OpenVX context which must be created using \ref vxCreateContext
 * \param [out] obj        VISS Module object which gets populated with VISS node data objects
 * \param [in]  sensorObj  Sensor Module object used to initialize VISS data object parameters;
 *                         must be initialized prior to passing to this function
 */
vx_status tiovx_viss_module_init(vx_context context, TIOVXVISSModuleObj *obj, SensorObj *sensorObj);

/** \brief VISS module deinit helper function
 *
 * This VISS deinit helper function will release all the data objects created during the \ref app_init_viss call
 *
 * \param [in,out] obj    VISS Module object which contains VISS node data objects which are released in this function
 *
 */
vx_status tiovx_viss_module_deinit(TIOVXVISSModuleObj *obj);

/** \brief VISS module delete helper function
 *
 * This VISS delete helper function will delete the VISS node and write node that is created during the \ref app_create_graph_viss call
 *
 * \param [in,out] obj   VISS Module object which contains VISS node objects which are released in this function
 *
 */
vx_status tiovx_viss_module_delete(TIOVXVISSModuleObj *obj);

/** \brief VISS module create helper function
 *
 * This VISS create helper function will create the node using all the data objects created during the \ref app_init_viss call.
 * Internally calls \ref app_create_graph_viss_write_output if en_out_viss_write is set
 *
 * \param [in]     graph          OpenVX graph that has been created using \ref vxCreateGraph and where the VISS node is created
 * \param [in,out] obj            VISS Module object which contains VISS node and write node which are created in this function
 * \param [in]     raw_image_arr  Raw image input object array to VISS node.  Must be created separately, typically passed from output of capture node
 * \param [in]     ae_awb_result_arr  AE/AWB result object array to VISS node.  Must be created separately, typically passed from output of 2A node
 *
 */
vx_status tiovx_viss_module_create(vx_graph graph, TIOVXVISSModuleObj *obj, vx_object_array raw_image_arr, vx_object_array ae_awb_result_arr, const char* target_string);

/** \brief VISS module release buffers helper function
 *
 * This VISS helper function will release the buffers alloted during vxVerifyGraph stage
 *
 * \param [in] obj  VISS Module object
 *
 */
vx_status tiovx_viss_module_release_buffers(TIOVXVISSModuleObj *obj);

/** \brief VISS module write output helper function
 *
 * This VISS create helper function will create the node for writing the VISS output
 *
 * \param [in]     graph  OpenVX graph
 * \param [in,out] obj    VISS Module object which contains write node which is created in this function
 *
 */
vx_status tiovx_viss_module_add_write_output_node(vx_graph graph, TIOVXVISSModuleObj *obj);

/** \brief VISS module write output helper function
 *
 * This VISS create helper function will create the node for writing the VISS output
 *
 * \param [in] obj           VISS Module object which contains the write node used in this function
 * \param [in] start_frame   Starting frame to write
 * \param [in] num_frames    Total number of frames to write
 * \param [in] num_skip      Number of VISS frames to skip writing
 *
 */
vx_status tiovx_viss_module_send_write_output_cmd(TIOVXVISSModuleObj *obj, vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip);

/* @} */

#ifdef __cplusplus
}
#endif

#endif
