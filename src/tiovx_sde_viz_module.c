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
#include "tiovx_sde_viz_module.h"

static vx_status tiovx_sde_viz_module_configure_params(vx_context context, TIOVXSdeVizModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    obj->config = vxCreateUserDataObject(context, "tivx_sde_disparity_vis_params_t", sizeof(tivx_sde_disparity_vis_params_t), &obj->params);
    status = vxGetStatus((vx_reference)obj->config);

    if(status != VX_SUCCESS)
    {
        status = vxSetReferenceName((vx_reference)obj->config, "Stereo_Visualize_Config");
    }

    if(status != VX_SUCCESS)
    {
        TIOVX_MODULE_ERROR("[SDE-MODULE] Unable to create config object! \n");
    }

    return status;
}

static vx_status tiovx_sde_viz_module_create_input(vx_context context, TIOVXSdeVizModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_image in_image;
    vx_int32 buf;

    if(obj->input.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[SDE-VIZ-MODULE] Input buffer queue depth %d greater than max supported %d!\n", obj->input.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
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
                TIOVX_MODULE_ERROR("[SDE-VIZ-MODULE] Unable to create input array! \n");
                break;
            }
            obj->input.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->input.arr[buf], 0);
        }

        vxReleaseImage(&in_image);
    }
    else
    {
        TIOVX_MODULE_ERROR("[SDE-VIZ-MODULE] Unable to create input image template! \n");
    }

    return status;
}

static vx_status tiovx_sde_viz_module_create_output(vx_context context, TIOVXSdeVizModuleObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_image out_image;
    vx_int32 buf;

    if(obj->output.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[SDE-VIZ-MODULE] Output buffer queue depth %d greater than max supported %d!\n", obj->output.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
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
                TIOVX_MODULE_ERROR("[SDE-VIZ-MODULE] Unable to create output array! \n");
            }

            obj->output.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->output.arr[buf], 0);
        }
        vxReleaseImage(&out_image);
    }
    else
    {
        TIOVX_MODULE_ERROR("[SDE-VIZ-MODULE] Unable to create output image template! \n");
    }

    return status;
}

vx_status tiovx_sde_viz_module_init(vx_context context, TIOVXSdeVizModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    TIOVX_MODULE_PRINTF("[SDE-VIZ-MODULE] Configuring params!\n");
    status = tiovx_sde_viz_module_configure_params(context, obj);

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_sde_viz_module_create_input(context, obj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_sde_viz_module_create_output(context, obj);
    }

    return status;
}

vx_status tiovx_sde_viz_module_deinit(TIOVXSdeVizModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_int32 buf;

    TIOVX_MODULE_PRINTF("[SDE-MODULE] Releasing config user data object!\n");
    status = vxReleaseUserDataObject(&obj->config);

    for(buf = 0; buf < obj->input.bufq_depth; buf++)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[SDE-VIZ-MODULE] Releasing input image handles!\n");
            status = vxReleaseImage(&obj->input.image_handle[buf]);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[SDE-VIZ-MODULE] Releasing input image arr!\n");
            status = vxReleaseObjectArray(&obj->input.arr[buf]);
        }
    }

    for(buf = 0; buf < obj->output.bufq_depth; buf++)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[SDE-VIZ-MODULE] Releasing output handle!\n");
            status = vxReleaseImage(&obj->output.image_handle[buf]);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[SDE-VIZ-MODULE] Releasing output image arr!\n");
            status = vxReleaseObjectArray(&obj->output.arr[buf]);
        }
    }

    return status;
}

vx_status tiovx_sde_viz_module_delete(TIOVXSdeVizModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    if(obj->node != NULL)
    {
        TIOVX_MODULE_PRINTF("[SDE-VIZ-MODULE] Releasing node reference!\n");
        status = vxReleaseNode(&obj->node);
    }

    return status;
}

vx_status tiovx_sde_viz_module_create(vx_graph graph, TIOVXSdeVizModuleObj *obj, vx_object_array input_arr, const char* target_string)
{
    vx_status status = VX_SUCCESS;

    vx_image input;
    vx_image output;

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

    obj->node = tivxSdeDisparityVisualizeNode (graph,
                                               obj->config,
                                               input,
                                               output);

    status = vxGetStatus((vx_reference)obj->node);

    if((vx_status)VX_SUCCESS == status)
    {
        vxSetNodeTarget(obj->node, VX_TARGET_STRING, target_string);

        vx_bool replicate[] = { vx_false_e, vx_true_e, vx_true_e };

        vxReplicateNode(graph, obj->node, replicate, 3);
    }
    else
    {
        TIOVX_MODULE_ERROR("[SDE-VIZ-MODULE] Unable to create node! \n");
    }

    if(input != NULL)
        vxReleaseImage(&input);

    if(output != NULL)
        vxReleaseImage(&output);

    return status;
}

vx_status tiovx_sde_viz_module_release_buffers(TIOVXSdeVizModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    void      *virtAddr[TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    vx_uint32   size[TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32   numEntries;
    vx_int32   bufq, ch;


    /* Free input disparity image handles */
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
                        TIOVX_MODULE_PRINTF("[SDE-VIZ-MODULE] Freeing input, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
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
                        TIOVX_MODULE_PRINTF("[SDE-VIZ-MODULE] Freeing output, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
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
