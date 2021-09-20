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
#include "ldc_lut_1920x1080.h"

static uint8_t  g_ldc_lut[] = LDC_LUT_1920_1080;

static vx_status configure_dcc_params(vx_context context, TIOVXLDCModuleObj *ldcObj, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    if(sensorObj->sensor_dcc_enabled)
    {
        int32_t dcc_buff_size;
        uint8_t * dcc_buf;
        vx_map_id dcc_buf_map_id;

        dcc_buff_size = appIssGetDCCSizeLDC(sensorObj->sensor_name, sensorObj->sensor_wdr_enabled);

        if (dcc_buff_size > 0)
        {
            ldcObj->dcc_config = vxCreateUserDataObject(context, "dcc_ldc", dcc_buff_size, NULL );
            status = vxGetStatus((vx_reference)ldcObj->dcc_config);

            if(status == VX_SUCCESS)
            {
                vxSetReferenceName((vx_reference)ldcObj->dcc_config, "ldc_node_dcc_config");

                vxMapUserDataObject(
                        ldcObj->dcc_config, 0,
                        dcc_buff_size,
                        &dcc_buf_map_id,
                        (void **)&dcc_buf,
                        VX_WRITE_ONLY,
                        VX_MEMORY_TYPE_HOST, 0);

                status = appIssGetDCCBuffLDC(sensorObj->sensor_name, sensorObj->sensor_wdr_enabled,  dcc_buf, dcc_buff_size);
                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[LDC-MODULE] Error getting DCC buffer \n");
                }
                vxUnmapUserDataObject(ldcObj->dcc_config, dcc_buf_map_id);
            }
            else
            {
                TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to create DCC config object! \n");
            }
        }
    }
    else
    {
        ldcObj->dcc_config = NULL;
    }

    return status;
}
static vx_status configure_mesh_params(vx_context context, TIOVXLDCModuleObj *ldcObj, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    vx_uint32 table_width_ds, table_height_ds;
    vx_imagepatch_addressing_t image_addr;
    vx_rectangle_t rect;

    table_width_ds = (((ldcObj->table_width / (1 << ldcObj->ds_factor)) + 1u) + 15u) & (~15u);
    table_height_ds = ((ldcObj->table_height / (1 << ldcObj->ds_factor)) + 1u);

    /* Mesh Image */
    ldcObj->mesh_img = vxCreateImage(context, table_width_ds, table_height_ds, VX_DF_IMAGE_U32);
    status = vxGetStatus((vx_reference)ldcObj->mesh_img);

    if(status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)ldcObj->mesh_img, "ldc_node_mesh_img");

        /* Copy Mesh table */
        rect.start_x = 0;
        rect.start_y = 0;
        rect.end_x = table_width_ds;
        rect.end_y = table_height_ds;

        image_addr.dim_x = table_width_ds;
        image_addr.dim_y = table_height_ds;
        image_addr.stride_x = 4u;
        image_addr.stride_y = table_width_ds * 4u;

        status = vxCopyImagePatch(ldcObj->mesh_img,
                                &rect, 0,
                                &image_addr,
                                g_ldc_lut,
                                VX_WRITE_ONLY,
                                VX_MEMORY_TYPE_HOST);
        if (status == VX_SUCCESS)
        {
            /* Mesh Parameters */
            memset(&ldcObj->mesh_params, 0, sizeof(tivx_vpac_ldc_mesh_params_t));

            tivx_vpac_ldc_mesh_params_init(&ldcObj->mesh_params);

            ldcObj->mesh_params.mesh_frame_width  = ldcObj->table_width;
            ldcObj->mesh_params.mesh_frame_height = ldcObj->table_height;
            ldcObj->mesh_params.subsample_factor  = ldcObj->ds_factor;

            ldcObj->mesh_config = vxCreateUserDataObject(context, "tivx_vpac_ldc_mesh_params_t", sizeof(tivx_vpac_ldc_mesh_params_t), NULL);
            status = vxGetStatus((vx_reference)ldcObj->mesh_config);

            if(status == VX_SUCCESS)
            {
                vxSetReferenceName((vx_reference)ldcObj->mesh_config, "ldc_node_mesh_config");

                status = vxCopyUserDataObject(ldcObj->mesh_config, 0,
                                    sizeof(tivx_vpac_ldc_mesh_params_t),
                                    &ldcObj->mesh_params,
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

static vx_status configure_region_params(vx_context context, TIOVXLDCModuleObj *ldcObj, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    /* Block Size parameters */
    ldcObj->region_params.out_block_width  = LDC_BLOCK_WIDTH;
    ldcObj->region_params.out_block_height = LDC_BLOCK_HEIGHT;
    ldcObj->region_params.pixel_pad        = LDC_PIXEL_PAD;

    ldcObj->region_config = vxCreateUserDataObject(context, "tivx_vpac_ldc_region_params_t", sizeof(tivx_vpac_ldc_region_params_t),  NULL);
    status = vxGetStatus((vx_reference)ldcObj->region_config);

    if(status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)ldcObj->region_config, "ldc_node_region_config");

        status = vxCopyUserDataObject(ldcObj->region_config, 0,
                            sizeof(tivx_vpac_ldc_region_params_t),
                            &ldcObj->region_params,
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

static vx_status configure_ldc_params(vx_context context, TIOVXLDCModuleObj *ldcObj, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    /* LDC Configuration */
    tivx_vpac_ldc_params_init(&ldcObj->params);
    ldcObj->params.luma_interpolation_type = 1;
    ldcObj->params.dcc_camera_id = sensorObj->sensorParams.dccId;

    ldcObj->config = vxCreateUserDataObject(context, "tivx_vpac_ldc_params_t", sizeof(tivx_vpac_ldc_params_t), NULL);
    status = vxGetStatus((vx_reference)ldcObj->config);

    if(status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)ldcObj->config, "ldc_node_config");

        status = vxCopyUserDataObject(ldcObj->config, 0,
                            sizeof(tivx_vpac_ldc_params_t),
                            &ldcObj->params,
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

static vx_status create_ldc_outputs(vx_context context, TIOVXLDCModuleObj *ldcObj, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    /* LDC Output image in NV12 format */
    vx_image output_img = vxCreateImage(context, ldcObj->table_width, ldcObj->table_height, VX_DF_IMAGE_NV12);
    status = vxGetStatus((vx_reference)output_img);
    if(status == VX_SUCCESS)
    {
        ldcObj->output_arr = vxCreateObjectArray(context, (vx_reference)output_img, sensorObj->num_cameras_enabled);
        vxReleaseImage(&output_img);

        status = vxGetStatus((vx_reference)ldcObj->output_arr);
        if(status != VX_SUCCESS)
        {
            TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to create output image array! \n");
        }
        else
        {
            vxSetReferenceName((vx_reference)ldcObj->output_arr, "ldc_node_output_arr");
        }
    }
    else
    {
        TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to create output image! \n");
    }

    if(ldcObj->en_out_ldc_write == 1)
    {
        char file_path[TIVX_FILEIO_FILE_PATH_LENGTH];
        char file_prefix[TIVX_FILEIO_FILE_PREFIX_LENGTH];

        strcpy(file_path, ldcObj->output_file_path);
        ldcObj->file_path   = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PATH_LENGTH);
        status = vxGetStatus((vx_reference)ldcObj->file_path);
        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)ldcObj->file_path, "ldc_write_node_file_path");

            vxAddArrayItems(ldcObj->file_path, TIVX_FILEIO_FILE_PATH_LENGTH, &file_path[0], 1);
        }
        else
        {
            priTIOVX_MODULE_ERRORntf("[LDC-MODULE] Unable to create file path object for writing outputs! \n");
        }

        strcpy(file_prefix, "ldc_output");
        ldcObj->file_prefix = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PREFIX_LENGTH);
        status = vxGetStatus((vx_reference)ldcObj->file_prefix);
        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)ldcObj->file_prefix, "ldc_write_node_file_prefix");

            vxAddArrayItems(ldcObj->file_prefix, TIVX_FILEIO_FILE_PREFIX_LENGTH, &file_prefix[0], 1);
        }
        else
        {
            TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to create file prefix object for writing outputs! \n");
        }

        ldcObj->write_cmd = vxCreateUserDataObject(context, "tivxFileIOWriteCmd", sizeof(tivxFileIOWriteCmd), NULL);
        status = vxGetStatus((vx_reference)ldcObj->write_cmd);
        if(status != VX_SUCCESS)
        {
            TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to create file write cmd object! \n");
        }
        else
        {
            vxSetReferenceName((vx_reference)ldcObj->write_cmd, "ldc_write_node_write_cmd");
        }
    }
    else
    {
        ldcObj->file_path   = NULL;
        ldcObj->file_prefix = NULL;
        ldcObj->write_node  = NULL;
        ldcObj->write_cmd   = NULL;
    }

    return status;
}
vx_status app_init_ldc(vx_context context, TIOVXLDCModuleObj *ldcObj, SensorObj *sensorObj, char *objName)
{
    vx_status status = VX_SUCCESS;

    ldcObj->table_width  = LDC_TABLE_WIDTH;
    ldcObj->table_height = LDC_TABLE_HEIGHT;
    ldcObj->ds_factor    = LDC_DS_FACTOR;

    status = configure_dcc_params(context, ldcObj, sensorObj);

    if(status == VX_SUCCESS)
    {
        status = configure_mesh_params(context, ldcObj, sensorObj);
    }

    if(status == VX_SUCCESS)
    {
        status = configure_region_params(context, ldcObj, sensorObj);
    }

    if(status == VX_SUCCESS)
    {
        status = configure_ldc_params(context, ldcObj, sensorObj);
    }

    if(status == VX_SUCCESS)
    {
        status = create_ldc_outputs(context, ldcObj, sensorObj);
    }

    return (status);
}

void app_deinit_ldc(TIOVXLDCModuleObj *ldcObj)
{
    vxReleaseUserDataObject(&ldcObj->config);
    vxReleaseUserDataObject(&ldcObj->region_config);
    vxReleaseUserDataObject(&ldcObj->mesh_config);

    vxReleaseImage(&ldcObj->mesh_img);
    vxReleaseObjectArray(&ldcObj->output_arr);

    if(ldcObj->dcc_config != NULL)
    {
        vxReleaseUserDataObject(&ldcObj->dcc_config);
    }
    if(ldcObj->en_out_ldc_write == 1)
    {
        vxReleaseArray(&ldcObj->file_path);
        vxReleaseArray(&ldcObj->file_prefix);
        vxReleaseUserDataObject(&ldcObj->write_cmd);
    }
}

void app_delete_ldc(TIOVXLDCModuleObj *ldcObj)
{
    if(ldcObj->node != NULL)
    {
        vxReleaseNode(&ldcObj->node);
    }
    if(ldcObj->write_node != NULL)
    {
        vxReleaseNode(&ldcObj->write_node);
    }
}

vx_status app_create_graph_ldc(vx_graph graph, TIOVXLDCModuleObj *ldcObj, vx_object_array input_arr, const char* target_string)
{
    vx_status status = VX_SUCCESS;

    vx_image input_img, output0_img, output1_img;

    if(input_arr != NULL)
    {
        input_img = (vx_image)vxGetObjectArrayItem(input_arr, 0);
    }
    else
    {
        input_img = (vx_image)vxGetObjectArrayItem(ldcObj->input.arr[0], 0);
    }

    if(ldcObj->output0.arr[0] != NULL)
    {
        output0_img = NULL;
    }
    else
    {
        output0_img = (vx_image)vxGetObjectArrayItem(ldcObj->output0.arr[0], 0);
    }

    if(ldcObj->output1.arr[0] != NULL)
    {
        output1_img = NULL;
    }
    else
    {
        output1_img = (vx_image)vxGetObjectArrayItem(ldcObj->output1.arr[0], 0);
    }

    ldcObj->node = tivxVpacLdcNode(graph,
                                   ldcObj->config,
                                   ldcObj->warp_matrix,
                                   ldcObj->region_prms,
                                   ldcObj->mesh_prms,
                                   ldcObj->mesh_img,
                                   ldcObj->dcc_config,
                                   input_img,
                                   output0_img,
                                   output1_img);

    vxReleaseImage(&input_img);
    vxReleaseImage(&output0_img);
    vxReleaseImage(&output1_img);

    status = vxGetStatus((vx_reference)ldcObj->node);
    if(status == VX_SUCCESS)
    {
        vxSetNodeTarget(ldcObj->node, VX_TARGET_STRING, target_string);
        vxSetReferenceName((vx_reference)ldcObj->node, "ldc_node");

        vx_bool replicate[] = { vx_false_e, vx_false_e, vx_false_e, vx_false_e, vx_false_e, vx_false_e, vx_true_e, vx_true_e, vx_false_e};
        vxReplicateNode(graph, ldcObj->node, replicate, 9);
    }
    else
    {
        TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to create LDC node ! \n");
    }

    if(ldcObj->en_out_ldc_write == 1)
    {
        status = app_create_graph_ldc_write_output(graph, ldcObj);
    }

    return status;
}

vx_status app_create_graph_ldc_write_output(vx_graph graph, TIOVXLDCModuleObj *ldcObj)
{
    vx_status status = VX_SUCCESS;

    vx_image output_img = (vx_image)vxGetObjectArrayItem(ldcObj->output0.arr[0], 0);
    ldcObj->write_node = tivxWriteImageNode(graph, output_img, ldcObj->file_path, ldcObj->file_prefix);
    vxReleaseImage(&output_img);

    status = vxGetStatus((vx_reference)ldcObj->write_node);
    if(status == VX_SUCCESS)
    {
        vxSetNodeTarget(ldcObj->write_node, VX_TARGET_STRING, TIVX_TARGET_A72_0);
        vxSetReferenceName((vx_reference)ldcObj->write_node, "ldc_write_node");

        vx_bool replicate[] = { vx_true_e, vx_false_e, vx_false_e};
        vxReplicateNode(graph, ldcObj->write_node, replicate, 3);
    }
    else
    {
        TIOVX_MODULE_ERROR("[LDC-MODULE] Unable to create node to write LDC output! \n");
    }

    return (status);
}

vx_status app_send_cmd_ldc_write_node(TIOVXLDCModuleObj *ldcObj, vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip)
{
    vx_status status = VX_SUCCESS;

    tivxFileIOWriteCmd write_cmd;

    write_cmd.start_frame = start_frame;
    write_cmd.num_frames = num_frames;
    write_cmd.num_skip = num_skip;

    status = vxCopyUserDataObject(ldcObj->write_cmd, 0, sizeof(tivxFileIOWriteCmd),\
                  &write_cmd, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

    if(status == VX_SUCCESS)
    {
        vx_reference refs[2];

        refs[0] = (vx_reference)ldcObj->write_cmd;

        status = tivxNodeSendCommand(ldcObj->write_node, TIVX_CONTROL_CMD_SEND_TO_ALL_REPLICATED_NODES,
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
