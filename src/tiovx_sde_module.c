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
#include "tiovx_sde_module.h"

static vx_status tiovx_sde_module_configure_params(vx_context context, TIOVXSdeModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    obj->config = vxCreateUserDataObject(context, "tivx_dmpac_sde_params_t", sizeof(tivx_dmpac_sde_params_t), &obj->params);
    status = vxGetStatus((vx_reference)obj->config);

    if(status != VX_SUCCESS)
    {
        status = vxSetReferenceName((vx_reference)obj->config, "Stereo_Config");
    }

    if(status != VX_SUCCESS)
    {
        TIOVX_MODULE_ERROR("[SDE-MODULE] Unable to create config object! \n");
    }

    return status;
}

static vx_status tiovx_sde_module_create_inputs(vx_context context, TIOVXSdeModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_image in_image;
    vx_int32 buf;

    if(obj->input_left.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[SDE-MODULE] Input buffer queue depth %d greater than max supported %d!\n", obj->input_left.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
        return VX_FAILURE;
    }

    if(obj->input_right.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[SDE-MODULE] Input buffer queue depth %d greater than max supported %d!\n", obj->input_left.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
        return VX_FAILURE;
    }

    for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
    {
        obj->input_left.arr[buf]  = NULL;
        obj->input_left.image_handle[buf]  = NULL;
        obj->input_right.arr[buf]  = NULL;
        obj->input_right.image_handle[buf]  = NULL;
    }

    in_image = vxCreateImage(context, obj->width, obj->height, obj->input_right.color_format);
    status = vxGetStatus((vx_reference)in_image);

    if((vx_status)VX_SUCCESS == status)
    {
        for(buf = 0; buf < obj->input_right.bufq_depth; buf++)
        {
            obj->input_right.arr[buf]  = vxCreateObjectArray(context, (vx_reference)in_image, obj->num_channels);

            status = vxGetStatus((vx_reference)obj->input_right.arr[buf]);
            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_ERROR("[SDE-MODULE] Unable to create input array! \n");
                break;
            }
            obj->input_right.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->input_right.arr[buf], 0);
        }

        vxReleaseImage(&in_image);
    }
    else
    {
        TIOVX_MODULE_ERROR("[SDE-MODULE] Unable to create right input image template! \n");
    }

    if((vx_status)VX_SUCCESS == status)
    {
        in_image = vxCreateImage(context, obj->width, obj->height, obj->input_left.color_format);
        status = vxGetStatus((vx_reference)in_image);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        for(buf = 0; buf < obj->input_left.bufq_depth; buf++)
        {
            obj->input_left.arr[buf]  = vxCreateObjectArray(context, (vx_reference)in_image, obj->num_channels);

            status = vxGetStatus((vx_reference)obj->input_left.arr[buf]);
            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_ERROR("[SDE-MODULE] Unable to create input array! \n");
                break;
            }
            obj->input_left.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->input_left.arr[buf], 0);
        }

        vxReleaseImage(&in_image);
    }
    else
    {
        TIOVX_MODULE_ERROR("[SDE-MODULE] Unable to create left input image template! \n");
    }

    return status;
}

static vx_status tiovx_sde_module_create_output(vx_context context, TIOVXSdeModuleObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_image out_image;
    vx_int32 buf;

    if(obj->output.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[SDE-MODULE] Output buffer queue depth %d greater than max supported %d!\n", obj->output.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
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
                TIOVX_MODULE_ERROR("[SDE-MODULE] Unable to create output array! \n");
            }

            obj->output.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->output.arr[buf], 0);
        }
        vxReleaseImage(&out_image);
    }
    else
    {
        TIOVX_MODULE_ERROR("[SDE-MODULE] Unable to create output image template! \n");
    }

    return status;
}

vx_status tiovx_sde_module_init(vx_context context, TIOVXSdeModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    TIOVX_MODULE_PRINTF("[SDE-MODULE] Configuring params!\n");
    status = tiovx_sde_module_configure_params(context, obj);

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_sde_module_create_inputs(context, obj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_sde_module_create_output(context, obj);
    }

    return status;
}

vx_status tiovx_sde_module_deinit(TIOVXSdeModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_int32 buf;

    TIOVX_MODULE_PRINTF("[SDE-MODULE] Releasing config user data object!\n");
    status = vxReleaseUserDataObject(&obj->config);

    for(buf = 0; buf < obj->input_left.bufq_depth; buf++)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[SDE-MODULE] Releasing input image handle!\n");
            status = vxReleaseImage(&obj->input_left.image_handle[buf]);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[SDE-MODULE] Releasing input image arr!\n");
            status = vxReleaseObjectArray(&obj->input_left.arr[buf]);
        }
    }

    for(buf = 0; buf < obj->input_right.bufq_depth; buf++)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[SDE-MODULE] Releasing input image handle!\n");
            status = vxReleaseImage(&obj->input_right.image_handle[buf]);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[SDE-MODULE] Releasing input image arr!\n");
            status = vxReleaseObjectArray(&obj->input_right.arr[buf]);
        }
    }

    for(buf = 0; buf < obj->output.bufq_depth; buf++)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[SDE-MODULE] Releasing output flow vector handle!\n");
            status = vxReleaseImage(&obj->output.image_handle[buf]);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[SDE-MODULE] Releasing output image arr!\n");
            status = vxReleaseObjectArray(&obj->output.arr[buf]);
        }
    }

    return status;
}

vx_status tiovx_sde_module_delete(TIOVXSdeModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    if(obj->node != NULL)
    {
        TIOVX_MODULE_PRINTF("[SDE-MODULE] Releasing node reference!\n");
        status = vxReleaseNode(&obj->node);
    }

    return status;
}

vx_status tiovx_sde_module_create(vx_graph graph, TIOVXSdeModuleObj *obj, vx_object_array input_left_arr, vx_object_array input_right_arr, const char* target_string)
{
    vx_status status = VX_SUCCESS;

    vx_image input_left, input_right;
    vx_image output;

    if(input_left_arr != NULL)
    {
        input_left = (vx_image)vxGetObjectArrayItem((vx_object_array)input_left_arr, 0);
    }
    else
    {
        if(obj->input_left.arr[0] != NULL)
        {
            input_left = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->input_left.arr[0], 0);
        }
        else
        {
            input_left = NULL;
        }
    }

    if(input_right_arr != NULL)
    {
        input_right = (vx_image)vxGetObjectArrayItem((vx_object_array)input_right_arr, 0);
    }
    else
    {
        if(obj->input_right.arr[0] != NULL)
        {
            input_right = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->input_right.arr[0], 0);
        }
        else
        {
            input_right = NULL;
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

    obj->node = tivxDmpacSdeNode(graph,
                                 obj->config,
                                 input_left,
                                 input_right,
                                 output,
                                 NULL);

    status = vxGetStatus((vx_reference)obj->node);

    if((vx_status)VX_SUCCESS == status)
    {
        vxSetNodeTarget(obj->node, VX_TARGET_STRING, target_string);

        vx_bool replicate[] = { vx_false_e, vx_true_e, vx_true_e, vx_true_e, vx_false_e };

        vxReplicateNode(graph, obj->node, replicate, 5);
    }
    else
    {
        TIOVX_MODULE_ERROR("[SDE-MODULE] Unable to create node! \n");
    }

    if(input_left != NULL)
        vxReleaseImage(&input_left);

    if(input_right != NULL)
        vxReleaseImage(&input_right);

    if(output != NULL)
        vxReleaseImage(&output);

    return status;
}

vx_status tiovx_sde_module_release_buffers(TIOVXSdeModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    void      *virtAddr[TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    vx_uint32   size[TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32   numEntries;
    vx_int32   bufq, ch;

    /* Free input image handles */
    for(bufq = 0; bufq < obj->input_left.bufq_depth; bufq++)
    {
        for(ch = 0; ch < obj->num_channels; ch++)
        {
            vx_reference ref = vxGetObjectArrayItem(obj->input_left.arr[bufq], ch);
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
                            TIOVX_MODULE_PRINTF("[SDE-MODULE] Freeing input, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)freePtr, freeSize);
                            tivxMemFree(freePtr, freeSize, TIVX_MEM_EXTERNAL);
                            freePtr = virtAddr[ctr];
                            freeSize = size[ctr];
                        }
                    }

                    //Free the last set
                    TIOVX_MODULE_PRINTF("[SDE-MODULE] Freeing input, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)freePtr, freeSize);
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

    /* Free ref input image handles */
    for(bufq = 0; bufq < obj->input_right.bufq_depth; bufq++)
    {
        for(ch = 0; ch < obj->num_channels; ch++)
        {
            vx_reference ref = vxGetObjectArrayItem(obj->input_right.arr[bufq], ch);
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
                            TIOVX_MODULE_PRINTF("[SDE-MODULE] Freeing input, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)freePtr, freeSize);
                            tivxMemFree(freePtr, freeSize, TIVX_MEM_EXTERNAL);
                            freePtr = virtAddr[ctr];
                            freeSize = size[ctr];
                        }
                    }

                    //Free the last set
                    TIOVX_MODULE_PRINTF("[SDE-MODULE] Freeing input, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)freePtr, freeSize);
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
                        TIOVX_MODULE_PRINTF("[SDE-MODULE] Freeing output, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
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
