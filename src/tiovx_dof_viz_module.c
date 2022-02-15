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
#include "tiovx_dof_viz_module.h"

static vx_status tiovx_dof_viz_module_create_input(vx_context context, TIOVXDofVizModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_image in_image;
    vx_int32 buf;

    if(obj->input.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[DOF-VIZ-MODULE] Input buffer queue depth %d greater than max supported %d!\n", obj->input.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
        return VX_FAILURE;
    }

    for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
    {
        obj->input.arr[buf]  = NULL;
        obj->input.image_handle[buf]  = NULL;
    }

    in_image = vxCreateImage(context, obj->width, obj->height, obj->input.color_format);
    status = vxGetStatus((vx_reference)in_image);

    if((vx_status)VX_SUCCESS == status)
    {
        for(buf = 0; buf < obj->input.bufq_depth; buf++)
        {
            obj->input.arr[buf]  = vxCreateObjectArray(context, (vx_reference)in_image, obj->num_channels);

            status = vxGetStatus((vx_reference)obj->input.arr[buf]);
            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_ERROR("[DOF-VIZ-MODULE] Unable to create input array! \n");
                break;
            }
            obj->input.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->input.arr[buf], 0);
        }

        vxReleaseImage(&in_image);
    }
    else
    {
        TIOVX_MODULE_ERROR("[DOF-VIZ-MODULE] Unable to create input image template! \n");
    }

    return status;
}

static vx_status tiovx_dof_viz_module_create_flow_vector_output(vx_context context, TIOVXDofVizModuleObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_image out_image;
    vx_int32 buf;

    if(obj->output.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[DOF-VIZ-MODULE] Output buffer queue depth %d greater than max supported %d!\n", obj->output.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
        return VX_FAILURE;
    }

    for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
    {
        obj->output.arr[buf]  = NULL;
        obj->output.image_handle[buf]  = NULL;
    }

    out_image  = vxCreateImage(context, obj->width, obj->height, obj->output.color_format);
    status = vxGetStatus((vx_reference)out_image);

    if(status == VX_SUCCESS)
    {
        for(buf = 0; buf < obj->output.bufq_depth; buf++)
        {
            obj->output.arr[buf]  = vxCreateObjectArray(context, (vx_reference)out_image, obj->num_channels);

            status = vxGetStatus((vx_reference)obj->output.arr[buf]);
            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_ERROR("[DOF-VIZ-MODULE] Unable to create output array! \n");
            }

            obj->output.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->output.arr[buf], 0);
        }
        vxReleaseImage(&out_image);
    }
    else
    {
        TIOVX_MODULE_ERROR("[DOF-VIZ-MODULE] Unable to create output image template! \n");
    }

    return status;
}

static vx_status tiovx_dof_viz_module_create_confidence_image_output(vx_context context, TIOVXDofVizModuleObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_image out_image;
    vx_int32 buf;

    if(obj->output_confidence_image.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[DOF-VIZ-MODULE] Output buffer queue depth %d greater than max supported %d!\n", obj->output_confidence_image.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
        return VX_FAILURE;
    }

    for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
    {
        obj->output_confidence_image.arr[buf]  = NULL;
        obj->output_confidence_image.image_handle[buf]  = NULL;
    }

    out_image  = vxCreateImage(context, obj->width, obj->height, obj->output_confidence_image.color_format);
    status = vxGetStatus((vx_reference)out_image);

    if(status == VX_SUCCESS)
    {
        for(buf = 0; buf < obj->output_confidence_image.bufq_depth; buf++)
        {
            obj->output_confidence_image.arr[buf]  = vxCreateObjectArray(context, (vx_reference)out_image, obj->num_channels);

            status = vxGetStatus((vx_reference)obj->output_confidence_image.arr[buf]);
            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_ERROR("[DOF-VIZ-MODULE] Unable to create output array! \n");
            }

            obj->output_confidence_image.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->output_confidence_image.arr[buf], 0);
        }
        vxReleaseImage(&out_image);
    }
    else
    {
        TIOVX_MODULE_ERROR("[DOF-VIZ-MODULE] Unable to create output image template! \n");
    }

    return status;
}

vx_status tiovx_dof_viz_module_init(vx_context context, TIOVXDofVizModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    /* creating vx_scaler reference for confidence_threshold */
    obj->confidence_threshold = vxCreateScalar(context, VX_TYPE_UINT32, &obj->confidence_threshold_value);
    status = vxGetStatus((vx_reference)(obj->confidence_threshold));

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_dof_viz_module_create_input(context, obj);
    }
    else
    {
        TIOVX_MODULE_ERROR("[DOF-VIZ-MODULE] Unable to create confidence_threshold template! \n");
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_dof_viz_module_create_flow_vector_output(context, obj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        obj->output_confidence_image.bufq_depth = 1;
        obj->output_confidence_image.color_format = VX_DF_IMAGE_U8;

        status = tiovx_dof_viz_module_create_confidence_image_output(context, obj);
    }

    return status;
}

vx_status tiovx_dof_viz_module_deinit(TIOVXDofVizModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_int32 buf;

    status = vxReleaseScalar(&obj->confidence_threshold);

    for(buf = 0; buf < obj->input.bufq_depth; buf++)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DOF-VIZ-MODULE] Releasing input image handles!\n");
            status = vxReleaseImage(&obj->input.image_handle[buf]);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DOF-VIZ-MODULE] Releasing input image arr!\n");
            status = vxReleaseObjectArray(&obj->input.arr[buf]);
        }
    }

    for(buf = 0; buf < obj->output.bufq_depth; buf++)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DOF-VIZ-MODULE] Releasing output flow vector handle!\n");
            status = vxReleaseImage(&obj->output.image_handle[buf]);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DOF-VIZ-MODULE] Releasing output flow vector image arr!\n");
            status = vxReleaseObjectArray(&obj->output.arr[buf]);
        }
    }

    for(buf = 0; buf < obj->output_confidence_image.bufq_depth; buf++)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DOF-VIZ-MODULE] Releasing output flow vector handle!\n");
            status = vxReleaseImage(&obj->output_confidence_image.image_handle[buf]);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[DOF-VIZ-MODULE] Releasing output flow vector image arr!\n");
            status = vxReleaseObjectArray(&obj->output_confidence_image.arr[buf]);
        }
    }

    return status;
}

vx_status tiovx_dof_viz_module_delete(TIOVXDofVizModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    if(obj->node != NULL)
    {
        TIOVX_MODULE_PRINTF("[DOF-VIZ-MODULE] Releasing node reference!\n");
        status = vxReleaseNode(&obj->node);
    }

    return status;
}

vx_status tiovx_dof_viz_module_create(vx_graph graph, TIOVXDofVizModuleObj *obj, vx_object_array input_arr, const char* target_string)
{
    vx_status status = VX_SUCCESS;

    vx_image input;
    vx_image output;
    vx_image output_confidence_image;

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
        output = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->output.arr[0], 0);
    }
    else
    {
        output = NULL;
    }

    if(obj->output_confidence_image.arr[0] != NULL)
    {
        output_confidence_image = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->output_confidence_image.arr[0], 0);
    }
    else
    {
        output_confidence_image = NULL;
    }

    obj->node = tivxDofVisualizeNode (graph,
                                      input,
                                      obj->confidence_threshold,
                                      output,
                                      output_confidence_image
                                      );

    status = vxGetStatus((vx_reference)obj->node);

    if((vx_status)VX_SUCCESS == status)
    {
        vxSetNodeTarget(obj->node, VX_TARGET_STRING, target_string);

        vx_bool replicate[] = { vx_true_e, vx_false_e, vx_true_e, vx_true_e };

        vxReplicateNode(graph, obj->node, replicate, 4);
    }
    else
    {
        TIOVX_MODULE_ERROR("[DOF-VIZ-MODULE] Unable to create node! \n");
    }

    if(input != NULL)
        vxReleaseImage(&input);

    if(output != NULL)
        vxReleaseImage(&output);

    if(output_confidence_image != NULL)
        vxReleaseImage(&output_confidence_image);

    return status;
}

vx_status tiovx_dof_viz_module_release_buffers(TIOVXDofVizModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    void      *virtAddr[TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    vx_uint32   size[TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32   numEntries;
    vx_int32   bufq, ch;


    /* Free input flow vector image handles */
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
                        TIOVX_MODULE_PRINTF("[DOF-VIZ-MODULE] Freeing input, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
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

    /* Free output flow vector handles */
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
                        TIOVX_MODULE_PRINTF("[DOF-VIZ-MODULE] Freeing output, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
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

    return status;
}
