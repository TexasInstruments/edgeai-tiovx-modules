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

#include "tiovx_ldc_module.h"
#include <tiovx_utils.h>

static vx_status tiovx_ldc_module_configure_dcc_params(vx_context context, TIOVXLDCModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    obj->dcc_config = NULL;

    FILE *fp = fopen(obj->dcc_config_file_path, "rb");
    if(fp == NULL)
    {
        TIOVX_MODULE_ERROR("Unable to open DCC config file %s!\n", obj->dcc_config_file_path);
        status = VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        int32_t dcc_buff_size;

        fseek(fp, 0L, SEEK_END);
        dcc_buff_size = (int32_t)ftell(fp);
        fseek(fp, 0L, SEEK_SET);

        if (dcc_buff_size > 0)
        {
            uint8_t * dcc_buf;
            vx_map_id dcc_buf_map_id;

            obj->dcc_config = vxCreateUserDataObject(context, "dcc_ldc", dcc_buff_size, NULL );
            status = vxGetStatus((vx_reference)obj->dcc_config);

            if((vx_status)VX_SUCCESS == status)
            {
                vxMapUserDataObject(
                        obj->dcc_config, 0,
                        dcc_buff_size,
                        &dcc_buf_map_id,
                        (void **)&dcc_buf,
                        VX_WRITE_ONLY,
                        VX_MEMORY_TYPE_HOST, 0);

                int32_t bytes_read = fread(dcc_buf, sizeof(uint8_t), dcc_buff_size, fp);

                if(bytes_read != dcc_buff_size)
                {
                    TIOVX_MODULE_ERROR("[LDC-MODULE] DCC config bytes read %d not matching bytes expected %d \n", bytes_read, dcc_buff_size);
                    status = VX_FAILURE;
                }

                vxUnmapUserDataObject(obj->dcc_config, dcc_buf_map_id);
            }
            else
            {
                TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to create DCC config object! \n");
            }
        }
    }

    return status;
}

static vx_status tiovx_ldc_module_configure_mesh_params(vx_context context, TIOVXLDCModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_uint32 table_width_ds, table_height_ds;

    table_width_ds = (((obj->table_width / (1 << obj->ds_factor)) + 1u) + 15u) & (~15u);
    table_height_ds = ((obj->table_height / (1 << obj->ds_factor)) + 1u);


    /* Mesh Image */
    obj->mesh_img = vxCreateImage(context, table_width_ds, table_height_ds, VX_DF_IMAGE_U32);
    status = vxGetStatus((vx_reference)obj->mesh_img);

    if((vx_status)VX_SUCCESS == status)
    {
        /* Read LUT file */
        status = readImage(obj->lut_file_path, obj->mesh_img);

        if (status == VX_SUCCESS)
        {
            /* Mesh Parameters */
            memset(&obj->mesh_params, 0, sizeof(tivx_vpac_ldc_mesh_params_t));

            tivx_vpac_ldc_mesh_params_init(&obj->mesh_params);

            obj->mesh_params.mesh_frame_width  = obj->table_width;
            obj->mesh_params.mesh_frame_height = obj->table_height;
            obj->mesh_params.subsample_factor  = obj->ds_factor;

            obj->mesh_config = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t", sizeof(tivx_vpac_ldc_mesh_params_t), NULL);
            status = vxGetStatus((vx_reference)obj->mesh_config);

            if((vx_status)VX_SUCCESS == status)
            {
                status = vxCopyUserDataObject(obj->mesh_config, 0,
                                    sizeof(tivx_vpac_ldc_mesh_params_t),
                                    &obj->mesh_params,
                                    VX_WRITE_ONLY,
                                    VX_MEMORY_TYPE_HOST);
                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to copy mesh params into buffer! \n");
                }
            }
            else
            {
                TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to create mesh config object! \n");
            }
        }
        else
        {
            TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to copy mesh image! \n");
        }
    }
    else
    {
        TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to create mesh image! \n");
    }

    return status;
}

static vx_status tiovx_ldc_module_configure_region_params(vx_context context, TIOVXLDCModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    /* Block Size parameters */
    obj->region_params.out_block_width  = obj->out_block_width;
    obj->region_params.out_block_height = obj->out_block_height;
    obj->region_params.pixel_pad        = obj->pixel_pad;

    obj->region_config = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t", sizeof(tivx_vpac_ldc_region_params_t),  NULL);
    status = vxGetStatus((vx_reference)obj->region_config);

    if((vx_status)VX_SUCCESS == status)
    {
        status = vxCopyUserDataObject(obj->region_config, 0,
                            sizeof(tivx_vpac_ldc_region_params_t),
                            &obj->region_params,
                            VX_WRITE_ONLY,
                            VX_MEMORY_TYPE_HOST);
        if(status != VX_SUCCESS)
        {
            TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to copy ldc region params to buffer! \n");
        }
    }
    else
    {
        TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to create region config object! \n");
    }

    return status;
}

static vx_status tiovx_ldc_module_configure_ldc_params(vx_context context, TIOVXLDCModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    SensorObj *sensorObj = obj->sensorObj;

    /* LDC Configuration */
    tivx_vpac_ldc_params_init(&obj->params);
    obj->params.init_x = obj->init_x;
    obj->params.init_y = obj->init_y;
    obj->params.luma_interpolation_type = 1;
    obj->params.dcc_camera_id = sensorObj->sensorParams.dccId;

    obj->config = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), NULL);
    status = vxGetStatus((vx_reference)obj->config);

    if((vx_status)VX_SUCCESS == status)
    {
        status = vxCopyUserDataObject(obj->config, 0,
                            sizeof(tivx_vpac_ldc_params_t),
                            &obj->params,
                            VX_WRITE_ONLY,
                            VX_MEMORY_TYPE_HOST);
        if(status != VX_SUCCESS)
        {
            TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to copy ldc config params into buffer! \n");
        }
    }
    else
    {
        TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to create ldc params object! \n");
    }

    return status;
}

static vx_status tiovx_ldc_module_create_input(vx_context context, TIOVXLDCModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    SensorObj *sensorObj = obj->sensorObj;

    vx_image in_img;
    vx_int32 buf;

    if(obj->input.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[LDC-MODULE] Input buffer queue depth %d greater than max supported %d!\n", obj->input.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
        return VX_FAILURE;
    }

    for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
    {
        obj->input.arr[buf]  = NULL;
        obj->input.image_handle[buf]  = NULL;
    }

    in_img  = vxCreateImage(context, obj->input.width, obj->input.height, obj->input.color_format);
    status = vxGetStatus((vx_reference)in_img);

    if((vx_status)VX_SUCCESS == status)
    {
        for(buf = 0; buf < obj->input.bufq_depth; buf++)
        {
            obj->input.arr[buf]  = vxCreateObjectArray(context, (vx_reference)in_img, sensorObj->num_cameras_enabled);

            status = vxGetStatus((vx_reference)obj->input.arr[buf]);
            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to create input array! \n");
                break;
            }
            obj->input.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->input.arr[buf], 0);
        }

        vxReleaseImage(&in_img);
    }
    else
    {
        TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to create input image template! \n");
    }

    return status;
}

static vx_status tiovx_ldc_module_create_outputs(vx_context context, TIOVXLDCModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    SensorObj *sensorObj = obj->sensorObj;

    vx_image out_img;
    vx_int32 buf;

    /* Create output0 object */
    if(obj->output0.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[LDC-MODULE] Output buffer queue depth %d greater than max supported %d!\n", obj->output0.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
        return VX_FAILURE;
    }

    for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
    {
        obj->output0.arr[buf]  = NULL;
        obj->output0.image_handle[buf]  = NULL;
    }

    out_img  = vxCreateImage(context, obj->output0.width, obj->output0.height, obj->output0.color_format);
    status = vxGetStatus((vx_reference)out_img);

    if((vx_status)VX_SUCCESS == status)
    {
        for(buf = 0; buf < obj->output0.bufq_depth; buf++)
        {
            obj->output0.arr[buf]  = vxCreateObjectArray(context, (vx_reference)out_img, sensorObj->num_cameras_enabled);

            status = vxGetStatus((vx_reference)obj->output0.arr[buf]);
            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to create output0 array! \n");
            }

            obj->output0.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->output0.arr[buf], 0);
        }
        vxReleaseImage(&out_img);
    }
    else
    {
        TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to create output0 image template! \n");
    }

    /* Create output1 object based on LDC module config*/
    if(obj->en_output1 == 1)
    {
        if(obj->output1.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
        {
            TIOVX_MODULE_ERROR("[LDC-MODULE] Output buffer queue depth %d greater than max supported %d!\n", obj->output1.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
            return VX_FAILURE;
        }

        for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
        {
            obj->output1.arr[buf]  = NULL;
            obj->output1.image_handle[buf]  = NULL;
        }

        out_img  = vxCreateImage(context, obj->output1.width, obj->output1.height, obj->output1.color_format);
        status = vxGetStatus((vx_reference)out_img);

        if((vx_status)VX_SUCCESS == status)
        {
            for(buf = 0; buf < obj->output1.bufq_depth; buf++)
            {
                obj->output1.arr[buf]  = vxCreateObjectArray(context, (vx_reference)out_img, obj->sensorObj->num_cameras_enabled);

                status = vxGetStatus((vx_reference)obj->output1.arr[buf]);
                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to create output1 array! \n");
                }

                obj->output1.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->output1.arr[buf], 0);
            }
            vxReleaseImage(&out_img);
        }
        else
        {
            TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to create output1 image  template! \n");
        }
    }
    else
    {
        for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
        {
            obj->output1.arr[buf]  = NULL;
            obj->output1.image_handle[buf]  = NULL;
        }
    }

    if(obj->en_out_image_write == 1)
    {
        char file_path[TIVX_FILEIO_FILE_PATH_LENGTH];
        char file_prefix[TIVX_FILEIO_FILE_PREFIX_LENGTH];

        strcpy(file_path, obj->output_file_path);
        obj->file_path   = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PATH_LENGTH);
        status = vxGetStatus((vx_reference)obj->file_path);
        if((vx_status)VX_SUCCESS == status)
        {
            vxAddArrayItems(obj->file_path, TIVX_FILEIO_FILE_PATH_LENGTH, &file_path[0], 1);
        }
        else
        {
            TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to create file path object for fileio!\n");
        }

        sprintf(file_prefix, "ldc_output0");
        obj->file_prefix = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PREFIX_LENGTH);
        status = vxGetStatus((vx_reference)obj->file_prefix);
        if((vx_status)VX_SUCCESS == status)
        {
            vxAddArrayItems(obj->file_prefix, TIVX_FILEIO_FILE_PREFIX_LENGTH, &file_prefix[0], 1);
        }
        else
        {
            TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to create file prefix object for output!\n");
        }

        obj->write_cmd = vxCreateUserDataObject(context, "tivxFileIOWriteCmd", sizeof(tivxFileIOWriteCmd), NULL);
        status = vxGetStatus((vx_reference)obj->write_cmd);
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to create write cmd object for output!\n");
        }
    }
    else
    {
        obj->file_path   = NULL;
        obj->file_prefix = NULL;
        obj->write_node  = NULL;
        obj->write_cmd   = NULL;
    }

    return status;
}

vx_status tiovx_ldc_module_init(vx_context context, TIOVXLDCModuleObj *obj, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    obj->sensorObj = sensorObj;

    if(obj->sensorObj == NULL)
    {
        TIOVX_MODULE_ERROR("Sensor Object handle is NULL!");
        status = VX_FAILURE;
    }

    obj->mesh_config   = NULL;
    obj->region_config = NULL;
    obj->mesh_img      = NULL;
    obj->warp_matrix   = NULL;
    obj->dcc_config    = NULL;

    if(obj->ldc_mode == TIOVX_MODULE_LDC_OP_MODE_DCC_DATA)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            status = tiovx_ldc_module_configure_dcc_params(context, obj);
        }
    }
    else if (obj->ldc_mode == TIOVX_MODULE_LDC_OP_MODE_MESH_IMAGE)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            status = tiovx_ldc_module_configure_mesh_params(context, obj);
        }

        if((vx_status)VX_SUCCESS == status)
        {
            status = tiovx_ldc_module_configure_region_params(context, obj);
        }
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_ldc_module_configure_ldc_params(context, obj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_ldc_module_create_input(context, obj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_ldc_module_create_outputs(context, obj);
    }

#if defined(ENABLE_DCC_TOOL)
    if((vx_status)VX_SUCCESS == status)
    {
        status = itt_register_object(context,
                                     &(obj->node),
                                     NULL,
                                     NULL,
                                     LDC);
    }
#endif
    
    return (status);
}

vx_status tiovx_ldc_module_deinit(TIOVXLDCModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_int32 buf;

    if((vx_status)VX_SUCCESS == status)
    {
        TIOVX_MODULE_PRINTF("[LDC-MODULE] Releasing LDC config handle!\n");
        status = vxReleaseUserDataObject(&obj->config);
    }

    if(((vx_status)VX_SUCCESS == status) && (obj->region_config != NULL))
    {
        TIOVX_MODULE_PRINTF("[LDC-MODULE] Releasing region config handle!\n");
        status = vxReleaseUserDataObject(&obj->region_config);
    }

    if(((vx_status)VX_SUCCESS == status) && (obj->mesh_config != NULL))
    {
        TIOVX_MODULE_PRINTF("[LDC-MODULE] Releasing mesh config handle!\n");
        status = vxReleaseUserDataObject(&obj->mesh_config);
    }

    if(((vx_status)VX_SUCCESS == status) && (obj->mesh_img != NULL))
    {
        TIOVX_MODULE_PRINTF("[LDC-MODULE] Releasing mesh image handle!\n");
        status = vxReleaseImage(&obj->mesh_img);
    }

    if(((vx_status)VX_SUCCESS == status) && (obj->dcc_config != NULL))
    {
        TIOVX_MODULE_PRINTF("[LDC-MODULE] Releasing DCC config handle!\n");
        status = vxReleaseUserDataObject(&obj->dcc_config);
    }

    for(buf = 0; buf < obj->input.bufq_depth; buf++)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[LDC-MODULE] Releasing input image handle!\n");
            status = vxReleaseImage(&obj->input.image_handle[buf]);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[LDC-MODULE] Releasing input image arr!\n");
            status = vxReleaseObjectArray(&obj->input.arr[buf]);
        }
    }

    for(buf = 0; buf < obj->output0.bufq_depth; buf++)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[LDC-MODULE] Releasing output0 image handle!\n");
            status = vxReleaseImage(&obj->output0.image_handle[buf]);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[LDC-MODULE] Releasing output0 image arr!\n");
            status = vxReleaseObjectArray(&obj->output0.arr[buf]);
        }
    }

    for(buf = 0; buf < obj->output1.bufq_depth; buf++)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[LDC-MODULE] Releasing output1 image handle!\n");
            status = vxReleaseImage(&obj->output1.image_handle[buf]);
        }
        if(((vx_status)VX_SUCCESS == status) && (obj->en_output1 == 1))
        {
            TIOVX_MODULE_PRINTF("[LDC-MODULE] Releasing output1 image arr!\n");
            status = vxReleaseObjectArray(&obj->output1.arr[buf]);
        }
    }

    if(obj->en_out_image_write == 1)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[LDC-MODULE] Releasing output path array!\n");
            status = vxReleaseArray(&obj->file_path);
        }

        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[LDC-MODULE] Releasing output file prefix array!\n");
            status = vxReleaseArray(&obj->file_prefix);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[LDC-MODULE] Releasing output write command object!\n");
            status = vxReleaseUserDataObject(&obj->write_cmd);
        }
    }

    return status;
}

vx_status tiovx_ldc_module_delete(TIOVXLDCModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    if(obj->node != NULL)
    {
        TIOVX_MODULE_PRINTF("[LDC-MODULE] Releasing node reference!\n");
        status = vxReleaseNode(&obj->node);
    }

    if(((vx_status)VX_SUCCESS == status) && (obj->write_node != NULL))
    {
        TIOVX_MODULE_PRINTF("[LDC-MODULE] Releasing write node reference!\n");
        status = vxReleaseNode(&obj->write_node);
    }

    return status;
}

vx_status tiovx_ldc_module_create(vx_graph graph, TIOVXLDCModuleObj *obj, vx_object_array input_arr, const char* target_string)
{
    vx_status status = VX_SUCCESS;

    vx_image input_img, output0_img, output1_img = NULL;

    if(input_arr != NULL)
    {
        input_img = (vx_image)vxGetObjectArrayItem(input_arr, 0);
    }
    else
    {
        input_img = (vx_image)vxGetObjectArrayItem(obj->input.arr[0], 0);
    }

    output0_img = (vx_image)vxGetObjectArrayItem(obj->output0.arr[0], 0);

    if(obj->en_output1 == 1)
    {
        output1_img = (vx_image)vxGetObjectArrayItem(obj->output1.arr[0], 0);
    }

    obj->node = tivxVpacLdcNode(graph,
                                obj->config,
                                obj->warp_matrix,
                                obj->region_config,
                                obj->mesh_config,
                                obj->mesh_img,
                                obj->dcc_config,
                                input_img,
                                output0_img,
                                output1_img);

    status = vxGetStatus((vx_reference)obj->node);
    if((vx_status)VX_SUCCESS == status)
    {
        vxSetNodeTarget(obj->node, VX_TARGET_STRING, target_string);

        vx_bool replicate[] = { vx_false_e, vx_false_e, vx_false_e, vx_false_e, vx_false_e, vx_false_e, vx_true_e, vx_true_e, vx_false_e};
        vxReplicateNode(graph, obj->node, replicate, 9);
    }
    else
    {
        TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to create LDC node ! \n");
    }

    vxReleaseImage(&input_img);
    vxReleaseImage(&output0_img);

    if(obj->en_output1 == 1)
    {
        vxReleaseImage(&output1_img);
    }

    if(obj->en_out_image_write == 1)
    {
        status = tiovx_ldc_module_add_write_output_node(graph, obj);
    }

#if defined(ENABLE_DCC_TOOL)
    status = itt_server_edge_ai_init();
#endif

    return status;
}

vx_status tiovx_ldc_module_release_buffers(TIOVXLDCModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    SensorObj *sensorObj = obj->sensorObj;

    void       *virtAddr[TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    vx_uint32   size[TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32   numEntries;
    vx_int32    bufq, ch;

    /* Free input handles */
    for(bufq = 0; bufq < obj->input.bufq_depth; bufq++)
    {
        for(ch = 0; ch < sensorObj->num_cameras_enabled; ch++)
        {
            vx_reference ref = vxGetObjectArrayItem(obj->input.arr[bufq], ch);
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

                    if(virtAddr[0] != NULL)
                    {
                        TIOVX_MODULE_PRINTF("[LDC-MODULE] Freeing input, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
                        tivxMemFree(virtAddr[0], freeSize, TIVX_MEM_EXTERNAL);
                    }

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

    /* Free output handles */
    for(bufq = 0; bufq < obj->output0.bufq_depth; bufq++)
    {
        for(ch = 0; ch < sensorObj->num_cameras_enabled; ch++)
        {
            vx_reference ref = vxGetObjectArrayItem(obj->output0.arr[bufq], ch);
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

                    if(virtAddr[0] != NULL)
                    {
                        TIOVX_MODULE_PRINTF("[LDC-MODULE] Freeing output, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
                        tivxMemFree(virtAddr[0], freeSize, TIVX_MEM_EXTERNAL);
                    }

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
        VX_PRINT(VX_ZONE_ERROR, "tivxReferenceExportHandle() failed.\n");
    }

    return status;
}

vx_status tiovx_ldc_module_add_write_output_node(vx_graph graph, TIOVXLDCModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_image output_img = (vx_image)vxGetObjectArrayItem(obj->output0.arr[0], 0);
    obj->write_node = tivxWriteImageNode(graph, output_img, obj->file_path, obj->file_prefix);
    vxReleaseImage(&output_img);

    status = vxGetStatus((vx_reference)obj->write_node);
    if((vx_status)VX_SUCCESS == status)
    {
        vxSetNodeTarget(obj->write_node, VX_TARGET_STRING, TIVX_TARGET_A72_0);

        vx_bool replicate[] = { vx_true_e, vx_false_e, vx_false_e};
        vxReplicateNode(graph, obj->write_node, replicate, 3);
    }
    else
    {
        TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to create node to write LDC output! \n");
    }

    return (status);
}

vx_status tiovx_ldc_module_send_write_output_cmd(TIOVXLDCModuleObj *obj, vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip)
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
            TIOVX_MODULE_ERROR("LDC Node send command failed!\n");
        }

        TIOVX_MODULE_PRINTF("LDC node send command success!\n");
    }

    return (status);
}
