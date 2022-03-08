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
#include "tiovx_dof_module.h"

static vx_status tiovx_dof_module_configure_params(vx_context context, TIOVXDofModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    obj->config = vxCreateUserDataObject(context, "tivx_dmpac_dof_params_t", sizeof(tivx_dmpac_dof_params_t), NULL);
    status = vxGetStatus((vx_reference)obj->config);

    if(status == VX_SUCCESS)
    {
        vxSetReferenceName((vx_reference)obj->config, "dof_node_config");

        status = vxCopyUserDataObject(obj->config, 0, sizeof(tivx_dmpac_dof_params_t),\
                    &obj->params, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
        if(status != VX_SUCCESS)
        {
            TIOVX_MODULE_ERROR("[DOF-MODULE] Unable to copy dof params! \n");
        }
    }
    else
    {
        TIOVX_MODULE_ERROR("[DOF-MODULE] Unable to create config object! \n");
    }

    return status;
}

static vx_status tiovx_dof_module_create_pyramid_input(vx_context context, TIOVXDofModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_pyramid in_pyramid;
    vx_int32 buf;

    if(obj->input.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[DOF-MODULE] Input buffer queue depth %d greater than max supported %d!\n", obj->input.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
        return VX_FAILURE;
    }

    for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
    {
        obj->input.arr[buf]  = NULL;
        obj->input.pyramid_handle[buf]  = NULL;
    }

    in_pyramid = vxCreatePyramid(context, obj->input.levels, obj->input.scale, obj->width, obj->height, obj->input.color_format);
    status = vxGetStatus((vx_reference)in_pyramid);

    if((vx_status)VX_SUCCESS == status)
    {
        for(buf = 0; buf < obj->input.bufq_depth; buf++)
        {
            obj->input.arr[buf]  = vxCreateObjectArray(context, (vx_reference)in_pyramid, obj->num_channels);

            status = vxGetStatus((vx_reference)obj->input.arr[buf]);
            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_ERROR("[DOF-MODULE] Unable to create input array! \n");
                break;
            }
            obj->input.pyramid_handle[buf] = (vx_pyramid)vxGetObjectArrayItem((vx_object_array)obj->input.arr[buf], 0);
        }

        vxReleasePyramid(&in_pyramid);
    }
    else
    {
        TIOVX_MODULE_ERROR("[DOF-MODULE] Unable to create input pyramid template! \n");
    }

    return status;
}

static vx_status tiovx_dof_module_create_pyramid_ref_input(vx_context context, TIOVXDofModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_pyramid in_pyramid;
    vx_int32 buf;

    if(obj->input_ref.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[DOF-MODULE] Input buffer queue depth %d greater than max supported %d!\n", obj->input.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
        return VX_FAILURE;
    }

    for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
    {
        obj->input_ref.arr[buf]  = NULL;
        obj->input_ref.pyramid_handle[buf]  = NULL;
    }

    in_pyramid  = vxCreatePyramid(context, obj->input_ref.levels, obj->input_ref.scale, obj->width, obj->height, obj->input_ref.color_format);
    status = vxGetStatus((vx_reference)in_pyramid);

    if((vx_status)VX_SUCCESS == status)
    {
        for(buf = 0; buf < obj->input_ref.bufq_depth; buf++)
        {
            obj->input_ref.arr[buf]  = vxCreateObjectArray(context, (vx_reference)in_pyramid, obj->num_channels);

            status = vxGetStatus((vx_reference)obj->input_ref.arr[buf]);
            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_ERROR("[DOF-MODULE] Unable to create ref input array! \n");
                break;
            }
            obj->input_ref.pyramid_handle[buf] = (vx_pyramid)vxGetObjectArrayItem((vx_object_array)obj->input_ref.arr[buf], 0);
        }

        vxReleasePyramid(&in_pyramid);
    }
    else
    {
        TIOVX_MODULE_ERROR("[DOF-MODULE] Unable to create ref input pyramid template! \n");
    }

    return status;
}

static vx_status tiovx_dof_module_create_flow_vector_input(vx_context context, TIOVXDofModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_image in_img;
    vx_int32 buf;

    if(obj->input_flow_vector.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[DOF-MODULE] Input buffer queue depth %d greater than max supported %d!\n", obj->input.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
        return VX_FAILURE;
    }

    for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
    {
        obj->input_flow_vector.arr[buf]  = NULL;
        obj->input_flow_vector.image_handle[buf]  = NULL;
    }

    if (obj->enable_temporal_predicton_flow_vector)
    {
        in_img  = vxCreateImage(context, obj->width, obj->height, obj->input_flow_vector.color_format);
        status = vxGetStatus((vx_reference)in_img);

        if((vx_status)VX_SUCCESS == status)
        {
            for(buf = 0; buf < obj->input_flow_vector.bufq_depth; buf++)
            {
                obj->input.arr[buf]  = vxCreateObjectArray(context, (vx_reference)in_img, obj->num_channels);

                status = vxGetStatus((vx_reference)obj->input.arr[buf]);
                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[DOF-MODULE] Unable to create input array! \n");
                    break;
                }
                obj->input_flow_vector.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->input_flow_vector.arr[buf], 0);
            }

            vxReleaseImage(&in_img);
        }
        else
        {
            TIOVX_MODULE_ERROR("[DOF-MODULE] Unable to create input flow vector template! \n");
        }
    }

    return status;
}

static vx_status tiovx_dof_module_create_flow_vector_output(vx_context context, TIOVXDofModuleObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_image out_image;
    vx_int32 buf;

    if(obj->output_flow_vector.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[DOF-MODULE] Output buffer queue depth %d greater than max supported %d!\n", obj->output_flow_vector.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
        return VX_FAILURE;
    }

    for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
    {
        obj->output_flow_vector.arr[buf]  = NULL;
        obj->output_flow_vector.image_handle[buf]  = NULL;
    }

    out_image  = vxCreateImage(context, obj->width, obj->height, obj->output_flow_vector.color_format);
    status = vxGetStatus((vx_reference)out_image);

    if(status == VX_SUCCESS)
    {
        for(buf = 0; buf < obj->output_flow_vector.bufq_depth; buf++)
        {
            obj->output_flow_vector.arr[buf]  = vxCreateObjectArray(context, (vx_reference)out_image, obj->num_channels);

            status = vxGetStatus((vx_reference)obj->output_flow_vector.arr[buf]);
            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_ERROR("[DOF-MODULE] Unable to create output array! \n");
            }

            obj->output_flow_vector.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->output_flow_vector.arr[buf], 0);
        }
        vxReleaseImage(&out_image);
    }
    else
    {
        TIOVX_MODULE_ERROR("[DOF-MODULE] Unable to create output image template! \n");
    }

    return status;
}

static vx_status tiovx_dof_module_create_distribution_output(vx_context context, TIOVXDofModuleObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_distribution out_dst;
    vx_int32 buf;

    if(obj->output_distribution.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[DOF-MODULE] Output buffer queue depth %d greater than max supported %d!\n", obj->output_distribution.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
        return VX_FAILURE;
    }

    for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
    {
        obj->output_distribution.arr[buf]  = NULL;
        obj->output_distribution.dst_handle[buf]  = NULL;
    }

    if (obj->enable_output_distribution)
    {
        out_dst  = vxCreateDistribution(context, obj->output_distribution.num_bins, obj->output_distribution.offset, obj->output_distribution.range);
        status = vxGetStatus((vx_reference)out_dst);

        if(status == VX_SUCCESS)
        {
            for(buf = 0; buf < obj->output_distribution.bufq_depth; buf++)
            {
                obj->output_distribution.arr[buf]  = vxCreateObjectArray(context, (vx_reference)out_dst, obj->num_channels);

                status = vxGetStatus((vx_reference)obj->output_distribution.arr[buf]);
                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[DOF-MODULE] Unable to create output array! \n");
                }

                obj->output_distribution.dst_handle[buf] = (vx_distribution)vxGetObjectArrayItem((vx_object_array)obj->output_distribution.arr[buf], 0);
            }
            vxReleaseDistribution(&out_dst);
        }
        else
        {
            TIOVX_MODULE_ERROR("[DOF-MODULE] Unable to create output distribution template! \n");
        }
    }

    return status;
}

vx_status tiovx_dof_module_init(vx_context context, TIOVXDofModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    TIOVX_MODULE_PRINTF("[DOF-MODULE] Configuring params!\n");
    status = tiovx_dof_module_configure_params(context, obj);

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_dof_module_create_pyramid_input(context, obj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_dof_module_create_pyramid_ref_input(context, obj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_dof_module_create_flow_vector_input(context, obj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_dof_module_create_flow_vector_output(context, obj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_dof_module_create_distribution_output(context, obj);
    }

    return status;
}

vx_status tiovx_dof_module_deinit(TIOVXDofModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_int32 buf;

    TIOVX_MODULE_PRINTF("[DOF-MODULE] Releasing config user data object!\n");
    status = vxReleaseUserDataObject(&obj->config);

    for(buf = 0; buf < obj->input.bufq_depth; buf++)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DOF-MODULE] Releasing input pyramid handles!\n");
            status = vxReleasePyramid(&obj->input.pyramid_handle[buf]);
            status = vxReleasePyramid(&obj->input_ref.pyramid_handle[buf]);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DOF-MODULE] Releasing input pyramid arr!\n");
            status = vxReleaseObjectArray(&obj->input.arr[buf]);
            status = vxReleaseObjectArray(&obj->input_ref.arr[buf]);
        }
    }

    if (obj->enable_temporal_predicton_flow_vector)
    {
        for(buf = 0; buf < obj->input_flow_vector.bufq_depth; buf++)
        {
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[DOF-MODULE] Releasing input flow vector handles!\n");
                status = vxReleaseImage(&obj->input_flow_vector.image_handle[buf]);
            }
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[DOF-MODULE] Releasing input flow vector arr!\n");
                status = vxReleaseObjectArray(&obj->input_flow_vector.arr[buf]);
            }
        }
    }

    for(buf = 0; buf < obj->output_flow_vector.bufq_depth; buf++)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DOF-MODULE] Releasing output flow vector handle!\n");
            status = vxReleaseImage(&obj->output_flow_vector.image_handle[buf]);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DOF-MODULE] Releasing output image arr!\n");
            status = vxReleaseObjectArray(&obj->output_flow_vector.arr[buf]);
        }
    }

    if (obj->enable_output_distribution)
    {
        for(buf = 0; buf < obj->output_distribution.bufq_depth; buf++)
        {
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[DOF-MODULE] Releasing output distribution handle!\n");
                status = vxReleaseDistribution(&obj->output_distribution.dst_handle[buf]);
            }
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[DOF-MODULE] Releasing output distribution arr!\n");
                status = vxReleaseObjectArray(&obj->output_distribution.arr[buf]);
            }
        }
    }

    return status;
}

vx_status tiovx_dof_module_delete(TIOVXDofModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    if(obj->node != NULL)
    {
        TIOVX_MODULE_PRINTF("[DOF-MODULE] Releasing node reference!\n");
        status = vxReleaseNode(&obj->node);
    }

    return status;
}

vx_status tiovx_dof_module_create(vx_graph graph, TIOVXDofModuleObj *obj, vx_object_array input_arr, vx_object_array input_ref_arr, vx_object_array input_flow_vector_arr, const char* target_string)
{
    vx_status status = VX_SUCCESS;

    vx_pyramid input, input_ref;
    vx_image input_flow_vector;
    vx_image output_flow_vector;
    vx_distribution output_distribution;

    if(input_arr != NULL)
    {
        input = (vx_pyramid)vxGetObjectArrayItem((vx_object_array)input_arr, 0);
    }
    else
    {
        if(obj->input.arr[0] != NULL)
        {
            input = (vx_pyramid)vxGetObjectArrayItem((vx_object_array)obj->input.arr[0], 0);
        }
        else
        {
            input = NULL;
        }
    }

    if(input_ref_arr != NULL)
    {
        input_ref = (vx_pyramid)vxGetObjectArrayItem((vx_object_array)input_ref_arr, 0);
    }
    else
    {
        if(obj->input_ref.arr[0] != NULL)
        {
            input_ref = (vx_pyramid)vxGetObjectArrayItem((vx_object_array)obj->input_ref.arr[0], 0);
        }
        else
        {
            input_ref = NULL;
        }
    }

    if(input_flow_vector_arr != NULL)
    {
        input_flow_vector = (vx_image)vxGetObjectArrayItem((vx_object_array)input_flow_vector_arr, 0);
    }
    else
    {
        if(obj->input_flow_vector.arr[0] != NULL)
        {
            input_flow_vector = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->input_flow_vector.arr[0], 0);
        }
        else
        {
            input_flow_vector = NULL;
        }
    }

    if(obj->output_flow_vector.arr[0] != NULL)
    {
        output_flow_vector = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->output_flow_vector.arr[0], 0);
    }
    else
    {
        output_flow_vector = NULL;
    }

    if(obj->output_distribution.arr[0] != NULL)
    {
        output_distribution = (vx_distribution)vxGetObjectArrayItem((vx_object_array)obj->output_distribution.arr[0], 0);
    }
    else
    {
        output_distribution = NULL;
    }

    obj->node = tivxDmpacDofNode(graph,
                                obj->config,
                                NULL,
                                NULL,
                                input,
                                input_ref,
                                input_flow_vector,
                                NULL,
                                NULL,
                                output_flow_vector,
                                output_distribution);

    status = vxGetStatus((vx_reference)obj->node);

    if((vx_status)VX_SUCCESS == status)
    {
        vxSetNodeTarget(obj->node, VX_TARGET_STRING, target_string);

        vx_bool replicate[] = { vx_false_e, vx_false_e, vx_false_e, vx_true_e, vx_true_e, vx_true_e, vx_false_e, vx_false_e, vx_true_e, vx_true_e };

        vxReplicateNode(graph, obj->node, replicate, 10);
    }
    else
    {
        TIOVX_MODULE_ERROR("[DOF-MODULE] Unable to create node! \n");
    }

    if(input != NULL)
        vxReleasePyramid(&input);

    if(input_ref != NULL)
        vxReleasePyramid(&input_ref);

    if(input_flow_vector != NULL)
        vxReleaseImage(&input_flow_vector);

    if(output_flow_vector != NULL)
        vxReleaseImage(&output_flow_vector);

    if(output_distribution != NULL)
        vxReleaseDistribution(&output_distribution);

    return status;
}

vx_status tiovx_dof_module_release_buffers(TIOVXDofModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    void      *virtAddr[TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    vx_uint32   size[TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32   numEntries;
    vx_int32   bufq, ch;

    /* Free input pyramid handles */
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
                    vx_uint32 freeSize = size[0];
                    void      *freePtr = virtAddr[0];
                    for(ctr = 1; ctr < numEntries; ctr++)
                    {
                        if (freePtr + freeSize == virtAddr[ctr])
                        {
                            freeSize += size[ctr];
                        }
                        else
                        {
                            TIOVX_MODULE_PRINTF("[DOF-MODULE] Freeing input, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)freePtr, freeSize);
                            tivxMemFree(freePtr, freeSize, TIVX_MEM_EXTERNAL);
                            freePtr = virtAddr[ctr];
                            freeSize = size[ctr];
                        }
                    }

                    //Free the last set
                    TIOVX_MODULE_PRINTF("[DOF-MODULE] Freeing input, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)freePtr, freeSize);
                    tivxMemFree(freePtr, freeSize, TIVX_MEM_EXTERNAL);

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

    /* Free ref input pyramid handles */
    for(bufq = 0; bufq < obj->input_ref.bufq_depth; bufq++)
    {
        for(ch = 0; ch < obj->num_channels; ch++)
        {
            vx_reference ref = vxGetObjectArrayItem(obj->input_ref.arr[bufq], ch);
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
                    vx_uint32 freeSize = size[0];
                    void      *freePtr = virtAddr[0];
                    for(ctr = 1; ctr < numEntries; ctr++)
                    {
                        if (freePtr + freeSize == virtAddr[ctr])
                        {
                            freeSize += size[ctr];
                        }
                        else
                        {
                            TIOVX_MODULE_PRINTF("[DOF-MODULE] Freeing input, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)freePtr, freeSize);
                            tivxMemFree(freePtr, freeSize, TIVX_MEM_EXTERNAL);
                            freePtr = virtAddr[ctr];
                            freeSize = size[ctr];
                        }
                    }

                    //Free the last set
                    TIOVX_MODULE_PRINTF("[DOF-MODULE] Freeing input, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)freePtr, freeSize);
                    tivxMemFree(freePtr, freeSize, TIVX_MEM_EXTERNAL);

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

    /* Free input flow vector image handles */
    if (obj->enable_temporal_predicton_flow_vector)
    {
        for(bufq = 0; bufq < obj->input_flow_vector.bufq_depth; bufq++)
        {
            for(ch = 0; ch < obj->num_channels; ch++)
            {
                vx_reference ref = vxGetObjectArrayItem(obj->input_flow_vector.arr[bufq], ch);
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
                            TIOVX_MODULE_PRINTF("[DOF-MODULE] Freeing input, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
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
    }

    /* Free output flow vector handles */
    for(bufq = 0; bufq < obj->output_flow_vector.bufq_depth; bufq++)
    {
        for(ch = 0; ch < obj->num_channels; ch++)
        {
            vx_reference ref = vxGetObjectArrayItem(obj->output_flow_vector.arr[bufq], ch);
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
                        TIOVX_MODULE_PRINTF("[DOF-MODULE] Freeing output, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
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

    /* Free output distribution handles */
    if (obj->enable_output_distribution)
    {
        for(bufq = 0; bufq < obj->output_distribution.bufq_depth; bufq++)
        {
            for(ch = 0; ch < obj->num_channels; ch++)
            {
                vx_reference ref = vxGetObjectArrayItem(obj->output_distribution.arr[bufq], ch);
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
                            TIOVX_MODULE_PRINTF("[DOF-MODULE] Freeing output, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
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
    }

    return status;
}
