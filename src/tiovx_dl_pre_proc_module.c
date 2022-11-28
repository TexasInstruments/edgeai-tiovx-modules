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
#include "tiovx_dl_pre_proc_module.h"

static vx_status tiovx_dl_pre_proc_module_create_config(vx_context context, TIOVXDLPreProcModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    tivxDLPreProcParams *params;
    vx_map_id map_id;

    obj->config = vxCreateUserDataObject(context, "tivxDLPreProcArmv8Params", sizeof(tivxDLPreProcArmv8Params), NULL );
    status = vxGetStatus((vx_reference)obj->config);

    if (VX_SUCCESS == status)
    {
        vxSetReferenceName((vx_reference)obj->config, "dl_pre_proc_config");

        vxMapUserDataObject(obj->config, 0, sizeof(tivxDLPreProcParams), &map_id,
                        (void **)&params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        memcpy(params, &obj->params, sizeof(tivxDLPreProcArmv8Params));

        vxUnmapUserDataObject(obj->config, map_id);
    }

        return status;
}

static vx_status tiovx_dl_pre_proc_module_create_input(vx_context context, TIOVXDLPreProcModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_image in_img;
    vx_int32 buf;

    if(obj->input.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[DL-PRE-PROC-MODULE] Input buffer queue depth %d greater than max supported %d!\n", obj->input.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
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
            obj->input.arr[buf]  = vxCreateObjectArray(context, (vx_reference)in_img, obj->num_channels);

            status = vxGetStatus((vx_reference)obj->input.arr[buf]);
            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_ERROR("[DL-PRE-PROC-MODULE] Unable to create input array! \n");
                break;
            }
            obj->input.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->input.arr[buf], 0);
        }

        vxReleaseImage(&in_img);
    }
    else
    {
        TIOVX_MODULE_ERROR("[DL-PRE-PROC-MODULE] Unable to create input image template! \n");
    }

    return status;
}

static vx_status tiovx_dl_pre_proc_module_create_output(vx_context context, TIOVXDLPreProcModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_size tensor_sizes[VX_TENSOR_NUMBER_OF_DIMS];
    vx_tensor out_tensor;
    vx_int32 buf, dim;

    if(obj->output.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[DL-PRE-PROC-MODULE] Output buffer queue depth %d greater than max supported %d!\n", obj->output.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
        return VX_FAILURE;
    }

    for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
    {
        obj->output.arr[buf]  = NULL;
        obj->output.tensor_handle[buf]  = NULL;
    }

    for(dim = 0; dim < obj->output.num_dims; dim++)
    {
        tensor_sizes[dim] = obj->output.dim_sizes[dim];
    }

    out_tensor  = vxCreateTensor(context, obj->output.num_dims, tensor_sizes, obj->output.datatype, 0);
    status = vxGetStatus((vx_reference)out_tensor);

    if(status == VX_SUCCESS)
    {
        for(buf = 0; buf < obj->output.bufq_depth; buf++)
        {
            obj->output.arr[buf]  = vxCreateObjectArray(context, (vx_reference)out_tensor, obj->num_channels);

            status = vxGetStatus((vx_reference)obj->output.arr[buf]);
            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_ERROR("[DL-PRE-PROC-MODULE] Unable to create output array! \n");
            }

            obj->output.tensor_handle[buf] = (vx_tensor)vxGetObjectArrayItem((vx_object_array)obj->output.arr[buf], 0);
        }
        vxReleaseTensor(&out_tensor);
    }
    else
    {
        TIOVX_MODULE_ERROR("[DL-PRE-PROC-MODULE] Unable to create output image template! \n");
    }

    if(obj->en_out_tensor_write == 1)
    {
        char file_path[TIVX_FILEIO_FILE_PATH_LENGTH];
        char file_prefix[TIVX_FILEIO_FILE_PREFIX_LENGTH];

        strcpy(file_path, obj->output_file_path);
        obj->file_path   = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PATH_LENGTH);
        status = vxGetStatus((vx_reference)obj->file_path);
        if(status == VX_SUCCESS)
        {
            vxAddArrayItems(obj->file_path, TIVX_FILEIO_FILE_PATH_LENGTH, &file_path[0], 1);
        }
        else
        {
            TIOVX_MODULE_ERROR("[DL-PRE-PROC-MODULE] Unable to create file path object for fileio!\n");
        }

        sprintf(file_prefix, "dl_pre_proc_output");
        obj->file_prefix = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PREFIX_LENGTH);
        status = vxGetStatus((vx_reference)obj->file_prefix);
        if(status == VX_SUCCESS)
        {
            vxAddArrayItems(obj->file_prefix, TIVX_FILEIO_FILE_PREFIX_LENGTH, &file_prefix[0], 1);
        }
        else
        {
            TIOVX_MODULE_ERROR("[DL-PRE-PROC-MODULE] Unable to create file prefix object for output!\n");
        }

        obj->write_cmd = vxCreateUserDataObject(context, "tivxFileIOWriteCmd", sizeof(tivxFileIOWriteCmd), NULL);
        status = vxGetStatus((vx_reference)obj->write_cmd);
        if(status == VX_SUCCESS)
        {
            TIOVX_MODULE_ERROR("[DL-PRE-PROC-MODULE] Unable to create write cmd object for output!\n");
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

vx_status tiovx_dl_pre_proc_module_init(vx_context context, TIOVXDLPreProcModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    status = tiovx_dl_pre_proc_module_create_config(context, obj);

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_dl_pre_proc_module_create_input(context, obj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_dl_pre_proc_module_create_output(context, obj);
    }

    return status;
}

vx_status tiovx_dl_pre_proc_module_deinit(TIOVXDLPreProcModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_int32 buf;

    TIOVX_MODULE_PRINTF("[DL-PRE-PROC-MODULE] Releasing config handle!\n");
    status = vxReleaseUserDataObject(&obj->config);

    for(buf = 0; buf < obj->input.bufq_depth; buf++)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-PRE-PROC-MODULE] Releasing input image handle!\n");
            status = vxReleaseImage(&obj->input.image_handle[buf]);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-PRE-PROC-MODULE] Releasing input image arr!\n");
            status = vxReleaseObjectArray(&obj->input.arr[buf]);
        }
    }

    for(buf = 0; buf < obj->output.bufq_depth; buf++)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-PRE-PROC-MODULE] Releasing output tensor handle!\n");
            status = vxReleaseTensor(&obj->output.tensor_handle[buf]);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-PRE-PROC-MODULE] Releasing output tensor arr!\n");
            status = vxReleaseObjectArray(&obj->output.arr[buf]);
        }
    }

    if(obj->en_out_tensor_write == 1)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-PRE-PROC-MODULE] Releasing output path array!\n");
            status = vxReleaseArray(&obj->file_path);
        }

        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-PRE-PROC-MODULE] Releasing scaler output file prefix array!\n");
            status = vxReleaseArray(&obj->file_prefix);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-PRE-PROC-MODULE] Releasing scaler output write command object!\n");
            status = vxReleaseUserDataObject(&obj->write_cmd);
        }
    }

    return status;
}

vx_status tiovx_dl_pre_proc_module_delete(TIOVXDLPreProcModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    if(obj->node != NULL)
    {
        TIOVX_MODULE_PRINTF("[DL-PRE-PROC-MODULE] Releasing node reference!\n");
        status = vxReleaseNode(&obj->node);
    }
    if(obj->write_node != NULL)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-PRE-PROC-MODULE] Releasing write node reference!\n");
            status = vxReleaseNode(&obj->write_node);
        }
    }

    return status;
}

vx_status tiovx_dl_pre_proc_module_create(vx_graph graph, TIOVXDLPreProcModuleObj *obj, vx_object_array input_arr, const char* target_string)
{
    vx_status status = VX_SUCCESS;

    vx_image input;
    vx_tensor output;

    if(input_arr != NULL)
    {
        input = (vx_image)vxGetObjectArrayItem((vx_object_array)input_arr, 0);
    }
    else
    {
        if(obj->input.arr[0] != NULL)
        {
            input = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->input.arr[0], 0);
        }
        else
        {
            input = NULL;
        }
    }

    if(obj->output.arr[0] != NULL)
    {
        output = (vx_tensor)vxGetObjectArrayItem((vx_object_array)obj->output.arr[0], 0);
    }
    else
    {
        output = NULL;
    }

    obj->node = tivxDLPreProcArmv8Node(graph, obj->config, input, output);
    status = vxGetStatus((vx_reference)obj->node);

    if((vx_status)VX_SUCCESS == status)
    {
        vxSetNodeTarget(obj->node, VX_TARGET_STRING, target_string);

        vx_bool replicate[] = { vx_false_e, vx_true_e, vx_true_e };

        vxReplicateNode(graph, obj->node, replicate, 3);

        if(obj->en_out_tensor_write == 1)
        {
            if(output != NULL)
            {
                status = tiovx_dl_pre_proc_module_add_write_output_node(graph, obj);
                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[DL-PRE-PROC-MODULE] Unable to create write node for output!\n");
                }
            }
        }
    }
    else
    {
        TIOVX_MODULE_ERROR("[DL-PRE-PROC-MODULE] Unable to create node! \n");
    }

    if(input != NULL)
        vxReleaseImage(&input);

    if(output != NULL)
        vxReleaseTensor(&output);

    return status;
}

vx_status tiovx_dl_pre_proc_module_release_buffers(TIOVXDLPreProcModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    void      *virtAddr[TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    vx_uint32   size[TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32   numEntries;
    vx_int32   bufq, ch;

    /* Free input handles */
    for(bufq = 0; bufq < obj->input.bufq_depth; bufq++)
    {
        for(ch = 0; ch < obj->num_channels; ch++)
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
                        TIOVX_MODULE_PRINTF("[DL-PRE-PROC-MODULE] Freeing input, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
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
    for(bufq = 0; bufq < obj->output.bufq_depth; bufq++)
    {
        for(ch = 0; ch < obj->num_channels; ch++)
        {
            vx_reference ref = vxGetObjectArrayItem(obj->output.arr[bufq], ch);
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
                        TIOVX_MODULE_PRINTF("[DL-PRE-PROC-MODULE] Freeing output, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
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

vx_status tiovx_dl_pre_proc_module_add_write_output_node(vx_graph graph, TIOVXDLPreProcModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    /* Need to improve this section, currently one write node can take only one image. */
    vx_tensor output_tensor = (vx_tensor)vxGetObjectArrayItem(obj->output.arr[0], 0);
    obj->write_node = tivxWriteTensorNode(graph, output_tensor, obj->file_path, obj->file_prefix);
    vxReleaseTensor(&output_tensor);

    status = vxGetStatus((vx_reference)obj->write_node);

    if((vx_status)VX_SUCCESS == status)
    {
        vxSetNodeTarget(obj->write_node, VX_TARGET_STRING, TIVX_TARGET_A72_0);

        vx_bool replicate[] = { vx_true_e, vx_false_e, vx_false_e};
        vxReplicateNode(graph, obj->write_node, replicate, 3);
    }
    else
    {
        TIOVX_MODULE_ERROR("[DL-PRE-PROC-MODULE] Unable to create fileio write node for storing outputs! \n");
    }

    return (status);
}

vx_status tiovx_dl_pre_proc_module_send_write_output_cmd(TIOVXDLPreProcModuleObj *obj, vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip)
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
            TIOVX_MODULE_ERROR("[DL-PRE-PROC-MODULE] write node send command failed!\n");
        }

        TIOVX_MODULE_PRINTF("[DL-PRE-PROC-MODULE] write node send command success!\n");
    }

    return (status);
}
