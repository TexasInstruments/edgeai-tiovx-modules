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
#ifndef _TIOVX_AEWB_MODULE
#define _TIOVX_AEWB_MODULE

/**
 * \defgroup group_tiovx_modules_aewb AEWB Node Module
 *
 * \brief This section contains module APIs for the AEWB node tivxAewbNode
 *
 * \ingroup group_tiovx_modules
 *
 * @{
 */
#include <TI/j7_imaging_aewb.h>

#include "tiovx_modules_common.h"
#include "tiovx_sensor_module.h"

/** \brief AEWB Module Data Structure
 *
 * Contains the data objects required to use tivxAewbNode
 *
 */
typedef struct {
    /*! AEWB node object */
    vx_node node;

    /*! AEWB node config param object array */
    vx_object_array config_arr;

    /*! AEWB node params structure to initialize config object */
    tivx_aewb_config_t params;

    /*! AEWB DCC config user data object */
    vx_user_data_object dcc_config;

    /*! AEWB histogram object array */
    vx_object_array histogram_arr;

    /*! AEWB output object array */
    vx_object_array aewb_output_arr[TIOVX_MODULES_MAX_BUFQ_DEPTH];

    /*! Handle to first instance of AEWB output object array */
    vx_user_data_object aewb_output_handle[TIOVX_MODULES_MAX_BUFQ_DEPTH];

    /*! AEWB input object array */
    vx_object_array aewb_input_arr[TIOVX_MODULES_MAX_BUFQ_DEPTH];

    /*! Handle to first instance of AEWB input object array */
    vx_user_data_object aewb_input_handle[TIOVX_MODULES_MAX_BUFQ_DEPTH];

    /*! Bufq depth of output */
    vx_int32 out_bufq_depth;

    /*! Bufq depth of input */
    vx_int32 in_bufq_depth;

    /*! AEWB node graph parameter index of output */
    vx_int32 output_graph_parameter_index;

    /*! AEWB node graph parameter index of input */
    vx_int32 input_graph_parameter_index;

    /*! Pointer to sensor object */
    SensorObj *sensorObj;

    /* These params are needed only for writing intermediate output */
    vx_array file_path;
    vx_array file_prefix;
    vx_node write_node;
    vx_user_data_object write_cmd;

    vx_char output_file_path[TIVX_FILEIO_FILE_PATH_LENGTH];

}TIOVXAEWBModuleObj;

vx_status tiovx_aewb_module_init(vx_context context, TIOVXAEWBModuleObj *obj);
vx_status tiovx_aewb_module_deinit(TIOVXAEWBModuleObj *obj);
vx_status tiovx_aewb_module_delete(TIOVXAEWBModuleObj *obj);
vx_status tiovx_aewb_module_create(vx_graph graph, TIOVXAEWBModuleObj *obj, vx_object_array h3a_stats_arr, const char* target_string);
vx_status tiovx_aewb_module_release_buffers(TIOVXAEWBModuleObj *obj);

vx_status tiovx_aewb_module_add_write_output_node(vx_graph graph,TIOVXAEWBModuleObj *obj);
vx_status tiovx_aewb_module_send_write_output_cmd(TIOVXAEWBModuleObj *obj, vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip);

#endif
