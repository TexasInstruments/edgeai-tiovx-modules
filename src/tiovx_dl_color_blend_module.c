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
#include "tiovx_dl_color_blend_module.h"

static vx_status tiovx_dl_color_blend_module_create_config(vx_context context, TIOVXDLColorBlendModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    tivxDLColorBlendArmv8Params *params;
    vx_map_id map_id;

    obj->config = vxCreateUserDataObject(context, "tivxDLColorBlendArmv8Params", sizeof(tivxDLColorBlendArmv8Params), NULL );
    status = vxGetStatus((vx_reference)obj->config);

    if (VX_SUCCESS == status)
    {
        vxSetReferenceName((vx_reference)obj->config, "dl_color_blend_config");

        vxMapUserDataObject(obj->config, 0, sizeof(tivxDLColorBlendArmv8Params), &map_id,
                        (void **)&params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        memcpy(params, &obj->params, sizeof(tivxDLColorBlendArmv8Params));

        vxUnmapUserDataObject(obj->config, map_id);
    }

        return status;
}

static vx_status tiovx_dl_color_blend_module_create_image_input(vx_context context, TIOVXDLColorBlendModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_image in_img;
    vx_int32 buf;

    if(obj->img_input.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[DL-COLOR-BLEND-MODULE] Input buffer queue depth %d greater than max supported %d!\n", obj->img_input.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
        return VX_FAILURE;
    }

    for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
    {
        obj->img_input.arr[buf]  = NULL;
        obj->img_input.image_handle[buf]  = NULL;
    }

    in_img  = vxCreateImage(context, obj->img_input.width, obj->img_input.height, obj->img_input.color_format);
    status = vxGetStatus((vx_reference)in_img);

    if((vx_status)VX_SUCCESS == status)
    {
        for(buf = 0; buf < obj->img_input.bufq_depth; buf++)
        {
            obj->img_input.arr[buf]  = vxCreateObjectArray(context, (vx_reference)in_img, obj->num_channels);

            status = vxGetStatus((vx_reference)obj->img_input.arr[buf]);
            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_ERROR("[DL-COLOR-BLEND-MODULE] Unable to create input array! \n");
                break;
            }
            obj->img_input.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->img_input.arr[buf], 0);
        }

        vxReleaseImage(&in_img);
    }
    else
    {
        TIOVX_MODULE_ERROR("[DL-COLOR-BLEND-MODULE] Unable to create input image template! \n");
    }

    return status;
}

static vx_status tiovx_dl_color_blend_module_create_tensor_input(vx_context context, TIOVXDLColorBlendModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_size tensor_sizes[VX_TENSOR_NUMBER_OF_DIMS];
    vx_tensor in_tensor;
    vx_int32 buf, dim;

    if(obj->tensor_input.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[DL-COLOR-BLEND-MODULE] Output buffer queue depth %d greater than max supported %d!\n", obj->tensor_input.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
        return VX_FAILURE;
    }

    for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
    {
        obj->tensor_input.arr[buf]  = NULL;
        obj->tensor_input.tensor_handle[buf]  = NULL;
    }

    for(dim = 0; dim < obj->tensor_input.num_dims; dim++)
    {
        tensor_sizes[dim] = obj->tensor_input.dim_sizes[dim];
    }

    in_tensor  = vxCreateTensor(context, obj->tensor_input.num_dims, tensor_sizes, obj->tensor_input.datatype, 0);
    status = vxGetStatus((vx_reference)in_tensor);

    if(status == VX_SUCCESS)
    {
        for(buf = 0; buf < obj->tensor_input.bufq_depth; buf++)
        {
            obj->tensor_input.arr[buf]  = vxCreateObjectArray(context, (vx_reference)in_tensor, obj->num_channels);

            status = vxGetStatus((vx_reference)obj->tensor_input.arr[buf]);
            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_ERROR("[DL-COLOR-BLEND-MODULE] Unable to create tensor input array! \n");
            }

            obj->tensor_input.tensor_handle[buf] = (vx_tensor)vxGetObjectArrayItem((vx_object_array)obj->tensor_input.arr[buf], 0);
        }
        vxReleaseTensor(&in_tensor);
    }
    else
    {
        TIOVX_MODULE_ERROR("[DL-COLOR-BLEND-MODULE] Unable to create tensor template! \n");
    }

    return status;
}

static vx_status tiovx_dl_color_blend_module_create_output(vx_context context, TIOVXDLColorBlendModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_image out_img;
    vx_int32 buf;

    if(obj->img_output.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[DL-COLOR-BLEND-MODULE] Output buffer queue depth %d greater than max supported %d!\n", obj->img_output.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
        return VX_FAILURE;
    }

    for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
    {
        obj->img_output.arr[buf]  = NULL;
        obj->img_output.image_handle[buf]  = NULL;
    }

    out_img  = vxCreateImage(context, obj->img_input.width, obj->img_input.height, obj->img_input.color_format);
    status = vxGetStatus((vx_reference)out_img);

    if(status == VX_SUCCESS)
    {
        for(buf = 0; buf < obj->img_output.bufq_depth; buf++)
        {
            obj->img_output.arr[buf]  = vxCreateObjectArray(context, (vx_reference)out_img, obj->num_channels);

            status = vxGetStatus((vx_reference)obj->img_output.arr[buf]);
            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_ERROR("[DL-COLOR-BLEND-MODULE] Unable to create output array! \n");
            }

            obj->img_output.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->img_output.arr[buf], 0);
        }
        vxReleaseImage(&out_img);
    }
    else
    {
        TIOVX_MODULE_ERROR("[DL-COLOR-BLEND-MODULE] Unable to create output image template! \n");
    }

    if(obj->en_out_image_write == 1)
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
            TIOVX_MODULE_ERROR("[DL-COLOR-BLEND-MODULE] Unable to create file path object for fileio!\n");
        }

        sprintf(file_prefix, "dl_color_blend_output");
        obj->file_prefix = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PREFIX_LENGTH);
        status = vxGetStatus((vx_reference)obj->file_prefix);
        if(status == VX_SUCCESS)
        {
            vxAddArrayItems(obj->file_prefix, TIVX_FILEIO_FILE_PREFIX_LENGTH, &file_prefix[0], 1);
        }
        else
        {
            TIOVX_MODULE_ERROR("[DL-COLOR-BLEND-MODULE] Unable to create file prefix object for output!\n");
        }

        obj->write_cmd = vxCreateUserDataObject(context, "tivxFileIOWriteCmd", sizeof(tivxFileIOWriteCmd), NULL);
        status = vxGetStatus((vx_reference)obj->write_cmd);
        if(status == VX_SUCCESS)
        {
            TIOVX_MODULE_ERROR("[DL-COLOR-BLEND-MODULE] Unable to create write cmd object for output!\n");
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

vx_status tiovx_dl_color_blend_module_init(vx_context context, TIOVXDLColorBlendModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    status = tiovx_dl_color_blend_module_create_config(context, obj);

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_dl_color_blend_module_create_image_input(context, obj);
    }
    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_dl_color_blend_module_create_tensor_input(context, obj);
    }
    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_dl_color_blend_module_create_output(context, obj);
    }

    return status;
}

vx_status tiovx_dl_color_blend_module_deinit(TIOVXDLColorBlendModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_int32 buf;

    TIOVX_MODULE_PRINTF("[DL-COLOR-BLEND-MODULE] Releasing config handle!\n");
    status = vxReleaseUserDataObject(&obj->config);

    for(buf = 0; buf < obj->img_input.bufq_depth; buf++)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-COLOR-BLEND-MODULE] Releasing input image handle!\n");
            status = vxReleaseImage(&obj->img_input.image_handle[buf]);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-COLOR-BLEND-MODULE] Releasing input image arr!\n");
            status = vxReleaseObjectArray(&obj->img_input.arr[buf]);
        }
    }

    for(buf = 0; buf < obj->tensor_input.bufq_depth; buf++)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-COLOR-BLEND-MODULE] Releasing input tensor handle!\n");
            status = vxReleaseTensor(&obj->tensor_input.tensor_handle[buf]);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-COLOR-BLEND-MODULE] Releasing input tensor arr!\n");
            status = vxReleaseObjectArray(&obj->tensor_input.arr[buf]);
        }
    }

    for(buf = 0; buf < obj->img_output.bufq_depth; buf++)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-COLOR-BLEND-MODULE] Releasing output image handle!\n");
            status = vxReleaseImage(&obj->img_output.image_handle[buf]);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-COLOR-BLEND-MODULE] Releasing output image arr!\n");
            status = vxReleaseObjectArray(&obj->img_output.arr[buf]);
        }
    }

    if(obj->en_out_image_write == 1)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-COLOR-BLEND-MODULE] Releasing output path array!\n");
            status = vxReleaseArray(&obj->file_path);
        }

        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-COLOR-BLEND-MODULE] Releasing color blend output file prefix array!\n");
            status = vxReleaseArray(&obj->file_prefix);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-COLOR-BLEND-MODULE] Releasing color blend output write command object!\n");
            status = vxReleaseUserDataObject(&obj->write_cmd);
        }
    }

    return status;
}

vx_status tiovx_dl_color_blend_module_delete(TIOVXDLColorBlendModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    if(obj->node != NULL)
    {
        TIOVX_MODULE_PRINTF("[DL-COLOR-BLEND-MODULE] Releasing node reference!\n");
        status = vxReleaseNode(&obj->node);
    }

    if(obj->write_node != NULL)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DL-COLOR-BLEND-MODULE] Releasing write node reference!\n");
            status = vxReleaseNode(&obj->write_node);
        }
    }

    return status;
}

vx_status tiovx_dl_color_blend_module_create(vx_graph graph, TIOVXDLColorBlendModuleObj *obj, vx_object_array img_input_arr, vx_object_array tensor_input_arr, const char* target_string)
{
    vx_status status = VX_SUCCESS;

    vx_image  img_input;
    vx_tensor tensor_input;
    vx_image  img_output;

    if(img_input_arr != NULL)
    {
        img_input = (vx_image)vxGetObjectArrayItem((vx_object_array)img_input_arr, 0);
    }
    else
    {
        if(obj->img_input.arr[0] != NULL)
        {
            img_input = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->img_input.arr[0], 0);
        }
        else
        {
            img_input = NULL;
        }
    }

    if(tensor_input_arr != NULL)
    {
        tensor_input = (vx_tensor)vxGetObjectArrayItem((vx_object_array)tensor_input_arr, 0);
    }
    else
    {
        if(obj->tensor_input.arr[0] != NULL)
        {
            tensor_input = (vx_tensor)vxGetObjectArrayItem((vx_object_array)obj->tensor_input.arr[0], 0);
        }
        else
        {
            tensor_input = NULL;
        }
    }

    if(obj->img_output.arr[0] != NULL)
    {
        img_output = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->img_output.arr[0], 0);
    }
    else
    {
        img_output = NULL;
    }

    obj->node = tivxDLColorBlendArmv8Node(graph, obj->config, img_input, tensor_input, img_output);
    status = vxGetStatus((vx_reference)obj->node);

    if((vx_status)VX_SUCCESS == status)
    {
        vxSetNodeTarget(obj->node, VX_TARGET_STRING, target_string);

        vx_bool replicate[8];

        replicate[0] = vx_false_e; /* config */
        replicate[1] = vx_true_e;  /* input image */
        replicate[2] = vx_true_e;  /* input tensor */
        replicate[3] = vx_true_e;  /* output image */

        vxReplicateNode(graph, obj->node, replicate, 4);

        if(obj->en_out_image_write == 1)
        {
            if(img_output != NULL)
            {
                status = tiovx_dl_color_blend_module_add_write_output_node(graph, obj);
                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[DL-COLOR-BLEND-MODULE] Unable to create write node for output!\n");
                }
            }
        }
    }
    else
    {
        TIOVX_MODULE_ERROR("[DL-COLOR-BLEND-MODULE] Unable to create node! \n");
    }

    if(img_input != NULL)
        vxReleaseImage(&img_input);

    if(tensor_input != NULL)
        vxReleaseTensor(&tensor_input);

    if(img_output != NULL)
        vxReleaseImage(&img_output);

    return status;
}

vx_status tiovx_dl_color_blend_module_release_buffers(TIOVXDLColorBlendModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    void      *virtAddr[TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    vx_uint32   size[TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32   numEntries;
    vx_int32   bufq, ch;

    /* Free image input handles */
    for(bufq = 0; bufq < obj->img_input.bufq_depth; bufq++)
    {
        for(ch = 0; ch < obj->num_channels; ch++)
        {
            vx_reference ref = vxGetObjectArrayItem(obj->img_input.arr[bufq], ch);
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
                        TIOVX_MODULE_PRINTF("[DL-COLOR-BLEND-MODULE] Freeing image input, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
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

    /* Free tensor input handles */
    for(bufq = 0; bufq < obj->tensor_input.bufq_depth; bufq++)
    {
        for(ch = 0; ch < obj->num_channels; ch++)
        {
            vx_reference ref = vxGetObjectArrayItem(obj->tensor_input.arr[bufq], ch);
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
                    vx_uint32 freeSize = 0;
                    for(ctr = 0; ctr < numEntries; ctr++)
                    {
                        freeSize += size[ctr];
                    }

                    if(virtAddr[0] != NULL)
                    {
                        TIOVX_MODULE_PRINTF("[DL-COLOR-BLEND-MODULE] Freeing tensor input, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
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
    for(bufq = 0; bufq < obj->img_output.bufq_depth; bufq++)
    {
        for(ch = 0; ch < obj->num_channels; ch++)
        {
            vx_reference ref = vxGetObjectArrayItem(obj->img_output.arr[bufq], ch);
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
                        TIOVX_MODULE_PRINTF("[DL-COLOR-BLEND-MODULE] Freeing image output, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
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

vx_status tiovx_dl_color_blend_module_add_write_output_node(vx_graph graph, TIOVXDLColorBlendModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    /* Need to improve this section, currently one write node can take only one image. */
    vx_tensor output_tensor = (vx_tensor)vxGetObjectArrayItem(obj->img_output.arr[0], 0);
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
        TIOVX_MODULE_ERROR("[DL-COLOR-BLEND-MODULE] Unable to create fileio write node for storing outputs! \n");
    }

    return (status);
}

vx_status tiovx_dl_color_blend_module_send_write_output_cmd(TIOVXDLColorBlendModuleObj *obj, vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip)
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
            TIOVX_MODULE_ERROR("[DL-COLOR-BLEND-MODULE] write node send command failed!\n");
        }

        TIOVX_MODULE_PRINTF("[DL-COLOR-BLEND-MODULE] write node send command success!\n");
    }

    return (status);
}
