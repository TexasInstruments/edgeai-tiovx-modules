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

#include "tiovx_aewb_module.h"


//DONE
static vx_status tiovx_aewb_module_configure_dcc(vx_context context, AEWBObj *aewbObj, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    if(sensorObj->sensor_dcc_enabled)
    {
        int32_t dcc_buff_size;
        uint8_t * dcc_buf;
        vx_map_id dcc_buf_map_id;

        dcc_buff_size = appIssGetDCCSize2A(sensorObj->sensor_name, sensorObj->sensor_wdr_enabled);
        if(dcc_buff_size < 0)
        {
            TIOVX_MODULE_PRINTF("[AEWB-MODULE] Invalid DCC size for 2A! \n");
            return VX_FAILURE;
        }
        aewbObj->dcc_config = vxCreateUserDataObject(context, "dcc_2a", dcc_buff_size, NULL);
        status = vxGetStatus((vx_reference)aewbObj->dcc_config);

        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)aewbObj->dcc_config, "aewb_node_dcc_config");

            vxMapUserDataObject(
                aewbObj->dcc_config,
                0,
                dcc_buff_size,
                &dcc_buf_map_id,
                (void **)&dcc_buf,
                VX_WRITE_ONLY,
                VX_MEMORY_TYPE_HOST,
                0
            );
            status = appIssGetDCCBuff2A(sensorObj->sensor_name, sensorObj->sensor_wdr_enabled,  dcc_buf, dcc_buff_size);
            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_PRINTF("[AEWB-MODULE] Error getting 2A DCC buffer! \n");
                return VX_FAILURE;
            }
            vxUnmapUserDataObject(aewbObj->dcc_config, dcc_buf_map_id);
        }
        else
        {
            TIOVX_MODULE_PRINTF("[AEWB-MODULE] Unable to create DCC config object!\n");
        }
    }
    else
    {
        aewbObj->dcc_config = NULL;
        printf("[AEWB-MODULE] Sensor DCC is disabled \n");
    }

    return status;
}

//DONE
static vx_status tiovx_aewb_module_configure_aewb(vx_context context, AEWBObj *aewbObj, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;
    vx_int32 ch;
    vx_int32 ch_mask;

    aewbObj->params.sensor_dcc_id       = sensorObj->sensorParams.dccId;
    aewbObj->params.sensor_img_format   = 0;
    aewbObj->params.sensor_img_phase    = 3;

    if(sensorObj->sensor_exp_control_enabled || sensorObj->sensor_gain_control_enabled )
    {
        aewbObj->params.ae_mode = ALGORITHMS_ISS_AE_AUTO;
    }
    else
    {
        aewbObj->params.ae_mode = ALGORITHMS_ISS_AE_DISABLED;
    }
    aewbObj->params.awb_mode = ALGORITHMS_ISS_AWB_AUTO;

    aewbObj->params.awb_num_skip_frames = 9;
    aewbObj->params.ae_num_skip_frames  = 9;
    aewbObj->params.channel_id          = 0;

    vx_user_data_object config = vxCreateUserDataObject(context, "tivx_aewb_config_t", sizeof(tivx_aewb_config_t), &aewbObj->params);
    status = vxGetStatus((vx_reference)config);
    if(status == VX_SUCCESS)
    {
        aewbObj->config_arr = vxCreateObjectArray(context, (vx_reference)config, sensorObj->num_cameras_enabled);
        status = vxGetStatus((vx_reference)aewbObj->config_arr);
        if(status != VX_SUCCESS)
        {
            TIOVX_MODULE_PRINTF("[AEWB_MODULE] Unable to create AEWB config object array! \n");
        }
        vxReleaseUserDataObject(&config);

        if(status == VX_SUCCESS)
        {
            vx_uint32 array_obj_index = 0;
            vxSetReferenceName((vx_reference)aewbObj->config_arr, "aewb_node_config_arr");

            ch = 0;
            ch_mask = sensorObj->ch_mask;
            while(ch_mask > 0)
            {
                if(ch_mask & 0x1)
                {
                    vx_user_data_object config = (vx_user_data_object)vxGetObjectArrayItem(aewbObj->config_arr, array_obj_index);
                    array_obj_index++;
	                aewbObj->params.channel_id = ch;
	                vxCopyUserDataObject(config, 0, sizeof(tivx_aewb_config_t), &aewbObj->params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
	                vxReleaseUserDataObject(&config);
                }
                ch++;
                ch_mask = ch_mask >> 1;
            }
        }
    }
    else
    {
        TIOVX_MODULE_PRINTF("[AEWB_MODULE] Unable to create AEWB config object! \n");
    }

    return status;
}

//DONE
static vx_status tiovx_aewb_module_create_histogram(vx_context context, AEWBObj *aewbObj, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    vx_distribution histogram = vxCreateDistribution(context, 256, 0, 256);
    status = vxGetStatus((vx_reference)histogram);
    if(status == VX_SUCCESS)
    {
        aewbObj->histogram_arr = vxCreateObjectArray(context, (vx_reference)histogram, sensorObj->num_cameras_enabled);
        status = vxGetStatus((vx_reference)aewbObj->histogram_arr);
        if(status != VX_SUCCESS)
        {
            TIOVX_MODULE_PRINTF("[AEWB_MODULE] Unable to create Histogram object array! \n");
        }
        else
        {
            vxSetReferenceName((vx_reference)aewbObj->histogram_arr, "aewb_node_histogram_arr");
        }
        vxReleaseDistribution(&histogram);
    }
    else
    {
        TIOVX_MODULE_PRINTF("[AEWB_MODULE] Unable to create Histogram object! \n");
    }

    return status;
}

//DONE
static vx_status tiovx_aewb_module_create_aewb_output(vx_context context, AEWBObj *aewbObj, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;
    vx_int32 q;

    vx_user_data_object aewb_output =  vxCreateUserDataObject(context, "tivx_ae_awb_params_t", sizeof(tivx_ae_awb_params_t), NULL);
    status = vxGetStatus((vx_reference)aewb_output);

    if(status == VX_SUCCESS)
    {
        for(q = 0; q < obj->out_bufq_depth; q++)
        {
            aewbObj->aewb_output_arr[q] = vxCreateObjectArray(context, (vx_reference)aewb_output, sensorObj->num_cameras_enabled);
            status = vxGetStatus((vx_reference)aewbObj->aewb_output_arr[q]);
            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_PRINTF("[AEWB_MODULE] Unable to create AEWB output object array! \n");
                break;
            }
            else
            {
                vx_char name[VX_MAX_REFERENCE_NAME];

                snprintf(name, VX_MAX_REFERENCE_NAME, "aewb_node_aewb_output_arr_%d", q);
                vxSetReferenceName((vx_reference)aewbObj->aewb_output_arr[q], name);
            }
        }
        vxReleaseUserDataObject(&aewb_output);
    }
    else
    {
        TIOVX_MODULE_PRINTF("[AEWB_MODULE] Unable to create AEWB output object! \n");
    }

    return status;
}

//DONE
static vx_status tiovx_aewb_module_create_h3a_input(vx_context context, AEWBObj  *aewbObj, SensorObj  *sensorObj)
{
    vx_status status = VX_SUCCESS;
    vx_int32 q;

    vx_user_data_object aewb_input =  vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL);
    status = vxGetStatus((vx_reference)aewb_input);
    if(status == VX_SUCCESS)
    {
        for(q = 0; q < obj->in_bufq_depth; q++)
        {
            aewbObj->aewb_input_arr[q] = vxCreateObjectArray(context, (vx_reference)aewb_input, sensorObj->num_cameras_enabled);
            status = vxGetStatus((vx_reference)aewbObj->aewb_input_arr[q]);
            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_PRINTF("[AEWB_MODULE] Unable to create AEWB input object array! \n");
                break;
            }
            else
            {
                vx_char name[VX_MAX_REFERENCE_NAME];

                snprintf(name, VX_MAX_REFERENCE_NAME, "aewb_node_aewb_input_arr_%d", q);
                vxSetReferenceName((vx_reference)aewbObj->aewb_input_arr[q], name);
            }
        }
        vxReleaseUserDataObject(&aewb_input);
    }
    else
    {
        TIOVX_MODULE_PRINTF("[AEWB_MODULE] Unable to create AEWB input object! \n");
    }

    return status;
};

//DONE
vx_status tiovx_aewb_module_init(vx_context context, AEWBObj *aewbObj)
{
    vx_status status = VX_SUCCESS;

    SensorObj *sensorObj;
    sensorObj = &aewbObj->sensorObj;

    if(status == VX_SUCCESS)
    {
        status = tiovx_aewb_module_configure_dcc(context, aewbObj, sensorObj);
    }

    if(status == VX_SUCCESS)
    {
        status = tiovx_aewb_module_create_h3a_input(context, aewbObj, sensorObj);
    }

    if(status == VX_SUCCESS)
    {
        status = tiovx_aewb_module_configure_aewb(context, aewbObj, sensorObj);
    }

    if(status == VX_SUCCESS)
    {
        status = tiovx_aewb_module_create_histogram(context, aewbObj, sensorObj);
    }

    if(status == VX_SUCCESS)
    {
        status = tiovx_aewb_module_create_aewb_output(context, aewbObj, sensorObj);
    }



    return (status);
}

//DONE
void tiovx_aewb_module_deinit(TIOVXAEWBModuleObj *aewbObj)
{
    vx_status status = VX_SUCCESS;
    vx_int32 q;

    if(aewbObj->dcc_config != NULL)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[AEWB-MODULE] Releasing dcc config reference!\n");
            status = vxReleaseUserDataObject(&aewbObj->dcc_config);
        }
    }

    for(q = 0; q < aewbObj->in_bufq_depth; q++)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[AEWB-MODULE] Releasing input reference bufq %d!\n", q);
            status = vxReleaseObjectArray(&aewbObj->aewb_input_arr[q]);
        }
    }

    if((vx_status)VX_SUCCESS == status)
    {
        TIOVX_MODULE_PRINTF("[AEWB-MODULE] Releasing config array reference!\n");
        status = vxReleaseObjectArray(&aewbObj->config_arr);

    }

    if((vx_status)VX_SUCCESS == status)
    {
        TIOVX_MODULE_PRINTF("[AEWB-MODULE] Releasing histogram array reference!\n");
        status = xReleaseObjectArray(&aewbObj->histogram_arr);

    }

    for(q = 0; q < aewbObj->out_bufq_depth; q++)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[AEWB-MODULE] Releasing output array reference bufq %d!\n", q);
            status = vxReleaseObjectArray(&aewbObj->aewb_output_arr[q]);

        }
    }
}

//DONE
void tiovx_aewb_module_delete(TIOVXAEWBModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    if(obj->node != NULL)
    {
        TIOVX_MODULE_PRINTF("[AEWB-MODULE] Releasing node reference!\n");
        status = vxReleaseNode(&obj->node);
    }

    if(obj->write_node != NULL)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[AEWB-MODULE] Releasing write node reference!\n");
            status = vxReleaseNode(&obj->write_node);
        }
    }

    return status;
}


//DONE
vx_status tiovx_aewb_module_create(vx_graph graph, TIOVXAEWBModuleObj *obj, vx_object_array h3a_stats_arr, const char* target_string)
{
    vx_status status = VX_SUCCESS;

    vx_user_data_object aewb_output;
    vx_user_data_object aewb_input;
    vx_user_data_object config;
    vx_distribution histogram;

    if(obj->aewb_output_arr != NULL){
        aewb_output = (vx_user_data_object)vxGetObjectArrayItem(obj->aewb_output_arr[0], 0);
    }
    else
    {
        aewb_output = NULL;
    }

    if(h3a_stats_arr != NULL){
        aewb_input   = (vx_user_data_object)vxGetObjectArrayItem(h3a_stats_arr, 0);
    }
    else
    {
        if(obj->aewb_input_arr != NULL){
            aewb_input   = (vx_user_data_object)vxGetObjectArrayItem(obj->aewb_input_arr[0], 0);
        }
        else
        {
            aewb_input = NULL;
        }
    }

    if(obj->config_arr != NULL)
    {
        config = (vx_user_data_object)vxGetObjectArrayItem(obj->config_arr, 0);
    }
    else
    {
        config = NULL;
    }

    if(obj->histogram_arr != NULL)
    {
        histogram  = (vx_distribution)vxGetObjectArrayItem(obj->histogram_arr, 0);
    }
    else
    {
        histogram = NULL;
    }

    obj->node = tivxAewbNode(graph,
                                 config,
                                 histogram,
                                 aewb_input,
                                 NULL,
                                 aewb_output,
                                 aewbObj->dcc_config);
    if(aewb_output != NULL)
        vxReleaseUserDataObject(&aewb_output);
    if(aewb_input != NULL)
        vxReleaseUserDataObject(&aewb_input);
    if(config != NULL)
        vxReleaseUserDataObject(&config);
    if(histogram != NULL)
        vxReleaseDistribution(&histogram);

    status = vxGetStatus((vx_reference)aewbObj->node);
    if(status != VX_SUCCESS)
    {
        TIOVX_MODULE_PRINTF("[AEWB_MODULE] Unable to create AEWB node! \n");
        return status;
    }

    vxSetNodeTarget(aewbObj->node, VX_TARGET_STRING, target_string);
    vxSetReferenceName((vx_reference)aewbObj->node, "aewb_node");

    vx_bool replicate[] = { vx_true_e, vx_true_e, vx_true_e, vx_false_e, vx_true_e, vx_false_e};
    vxReplicateNode(graph, aewbObj->node, replicate, 6);


    return status;
}

//DONE
vx_status tiovx_aewb_module_release_buffers(TIOVXAEWBModuleObj *obj)
{
    vx_status status = VX_SUCCESS;
    void      *virtAddr[TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    vx_uint32   size[TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32   numEntries;
    vx_int32 in, bufq;


    /* Free input */
    for(bufq = 0; bufq < obj->out_bufq_depth; bufq++)
    {
        for(in=0; in < obj->sensorObj.num_cameras_enabled; in++)
        {
            vx_reference ref = vxGetObjectArrayItem(obj->aewb_input_arr[bufq], in);
            status = vxGetStatus((vx_reference)ref);

            if((vx_status)VX_SUCCESS == status)
            {
                /* Export handles to get valid size information. */
                status = tivxReferenceExportHandle(ref,
                                                virtAddr,
                                                size,
                                                TIOVX_MODULES_MAX_REF_HANDLES,
                                                &numEntries);

                if((vx_status)VX_SUCCESS == status)
                {
                    vx_int32 ctr;
                    /* Currently the vx_image buffers are alloated in one shot for multiple planes.
                        So if we are freeing this buffer then we need to get only the first plane
                        pointer address but add up the all the sizes to free the entire buffer */
                    vx_uint32 freeSize = 0;
                    for(ctr = 0; ctr < numEntries; ctr++)
                    {
                        freeSize += size[ctr];
                    }

                    TIOVX_MODULE_PRINTF("[AEWB-MODULE] Freeing input %d, bufq=%d, addr = 0x%016lX, size = %d \n", in, bufq, (vx_uint64)virtAddr[0], freeSize);
                    tivxMemFree(virtAddr[0], freeSize, TIVX_MEM_EXTERNAL);

                    for(ctr = 0; ctr < numEntries; ctr++)
                    {
                        virtAddr[ctr] = NULL;
                    }

                    /* Assign NULL handles to the OpenVx objects as it will avoid
                        doing a tivxMemFree twice, once now and once during release */
                    status = tivxReferenceImportHandle(ref,
                                                    (const void **)virtAddr,
                                                    (const uint32_t *)size,
                                                    numEntries);
                }
                vxReleaseReference(&ref);
            }
        }
    }


    
    /* Free output */
    for(bufq = 0; bufq < obj->out_bufq_depth; bufq++)
    {
        for(in=0; in < obj->sensorObj.num_cameras_enabled; in++)
        {
            vx_reference ref = vxGetObjectArrayItem(obj->aewb_output_arr[bufq], in);
            status = vxGetStatus((vx_reference)ref);

            if((vx_status)VX_SUCCESS == status)
            {
                /* Export handles to get valid size information. */
                status = tivxReferenceExportHandle(ref,
                                                virtAddr,
                                                size,
                                                TIOVX_MODULES_MAX_REF_HANDLES,
                                                &numEntries);

                if((vx_status)VX_SUCCESS == status)
                {
                    vx_int32 ctr;
                    /* Currently the vx_image buffers are alloated in one shot for multiple planes.
                        So if we are freeing this buffer then we need to get only the first plane
                        pointer address but add up the all the sizes to free the entire buffer */
                    vx_uint32 freeSize = 0;
                    for(ctr = 0; ctr < numEntries; ctr++)
                    {
                        freeSize += size[ctr];
                    }

                    TIOVX_MODULE_PRINTF("[AEWB-MODULE] Freeing output %d, bufq=%d, addr = 0x%016lX, size = %d \n", in, bufq, (vx_uint64)virtAddr[0], freeSize);
                    tivxMemFree(virtAddr[0], freeSize, TIVX_MEM_EXTERNAL);

                    for(ctr = 0; ctr < numEntries; ctr++)
                    {
                        virtAddr[ctr] = NULL;
                    }

                    /* Assign NULL handles to the OpenVx objects as it will avoid
                        doing a tivxMemFree twice, once now and once during release */
                    status = tivxReferenceImportHandle(ref,
                                                    (const void **)virtAddr,
                                                    (const uint32_t *)size,
                                                    numEntries);
                }
                vxReleaseReference(&ref);
            }
        }
    }

    if ((vx_status)VX_SUCCESS != status)
    {
        TIOVX_MODULE_ERROR("[AEWB-MODULE] tivxReferenceExportHandle() failed.\n");
    }

    return status;
};

//TODO do I need "out" parameter like multi scaler for multiple sensors? Or is the assumption that we have one sensor obj in struct?
vx_status tiovx_aewb_module_add_write_output_node(vx_graph graph,TIOVXAEWBModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    /* Need to improve this section, currently one write node can take only one image. */
    vx_user_data_object output_user_data = (vx_user_data_object)vxGetObjectArrayItem(obj->aewb_output_arr[0], 0);
    obj->write_node = tivxWriteUserDataObjectNode(graph, output_user_data, obj->file_path, obj->file_prefix);
    vxReleaseUserDataObject(&output_user_data);

    status = vxGetStatus((vx_reference)obj->write_node);

    if((vx_status)VX_SUCCESS == status)
    {
        vxSetNodeTarget(obj->write_node, VX_TARGET_STRING, TIVX_TARGET_A72_0);

        vx_bool replicate[] = { vx_true_e, vx_false_e, vx_false_e};
        vxReplicateNode(graph, obj->write_node, replicate, 3);
    }
    else
    {
        TIOVX_MODULE_ERROR("[AEWB-MODULE] Unable to create fileio write node for storing outputs! \n");
    }

    return (status);
};

//DONE
vx_status tiovx_aewb_module_send_write_output_cmd(TIOVXAEWBModuleObj *obj, vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip)
{
    vx_status status = VX_SUCCESS;

    tivxFileIOWriteCmd write_cmd;

    write_cmd.start_frame = start_frame;
    write_cmd.num_frames = num_frames;
    write_cmd.num_skip = num_skip;

    status = vxCopyUserDataObject(obj->write_cmd, 0, sizeof(tivxFileIOWriteCmd),\
                &write_cmd, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

    if((vx_status)VX_SUCCESS == status)
    {
        vx_reference refs[2];

        refs[0] = (vx_reference)obj->write_cmd;

        status = tivxNodeSendCommand(obj->write_node, TIVX_CONTROL_CMD_SEND_TO_ALL_REPLICATED_NODES,
                                TIVX_FILEIO_CMD_SET_FILE_WRITE,
                                refs, 1u);

        if(VX_SUCCESS != status)
        {
            TIOVX_MODULE_ERROR("[AEWB-MODULE] write node send command failed!\n");
        }

        TIOVX_MODULE_PRINTF("[AEWB-MODULE] write node send command success!\n");
    }

    return (status);
};
