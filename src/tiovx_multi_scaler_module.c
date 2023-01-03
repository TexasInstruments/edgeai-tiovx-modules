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
#include "tiovx_multi_scaler_module.h"

static vx_status tiovx_multi_scaler_module_configure_scaler_coeffs(vx_context context, TIOVXMultiScalerModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    tivx_vpac_msc_coefficients_t coeffs;

    tiovx_multi_scaler_module_set_coeff(&coeffs, obj->interpolation_method);

    /* Set Coefficients */
    obj->coeff_obj = vxCreateUserDataObject(context,
                                "tivx_vpac_msc_coefficients_t",
                                sizeof(tivx_vpac_msc_coefficients_t),
                                NULL);
    status = vxGetStatus((vx_reference)obj->coeff_obj);

    if((vx_status)VX_SUCCESS == status)
    {
        vxSetReferenceName((vx_reference)obj->coeff_obj, "multi_scaler_node_coeff_obj");

        status = vxCopyUserDataObject(obj->coeff_obj, 0,
                                    sizeof(tivx_vpac_msc_coefficients_t),
                                    &coeffs,
                                    VX_WRITE_ONLY,
                                    VX_MEMORY_TYPE_HOST);
    }
    else
    {
        TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Unable to create scaler coeffs object! \n");
    }

    return status;
}

static vx_status tiovx_multi_scaler_module_configure_crop_params(vx_context context, TIOVXMultiScalerModuleObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_int32 out;

    for (out = 0; out < obj->num_outputs; out++)
    {
        obj->crop_obj[out] = vxCreateUserDataObject(context,
                "tivx_vpac_msc_crop_params_t",
                sizeof(tivx_vpac_msc_crop_params_t),
                NULL);

        status = vxGetStatus((vx_reference)obj->crop_obj[out]);

        if((vx_status)VX_SUCCESS == status)
        {
            status = vxCopyUserDataObject(obj->crop_obj[out], 0,
                    sizeof(tivx_vpac_msc_crop_params_t),
                    obj->crop_params + out,
                    VX_WRITE_ONLY,
                    VX_MEMORY_TYPE_HOST);
        }

        if((vx_status)VX_SUCCESS != status)
        {
            TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Creating user data object for crop params failed!, %d\n", out);
        }
    }

    return status;
}

static vx_status tiovx_multi_scaler_module_create_scaler_input(vx_context context, TIOVXMultiScalerModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_image in_img;
    vx_int32 buf;

    if(obj->input.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Input buffer queue depth %d greater than max supported %d!\n", obj->input.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
        return VX_FAILURE;
    }

    for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
    {
        obj->input.arr[buf]  = NULL;
        obj->input.image_handle[buf]  = NULL;
    }

    in_img  = vxCreateImage(context, obj->input.width, obj->input.height, obj->color_format);
    status = vxGetStatus((vx_reference)in_img);

    if((vx_status)VX_SUCCESS == status)
    {
        for(buf = 0; buf < obj->input.bufq_depth; buf++)
        {
            obj->input.arr[buf]  = vxCreateObjectArray(context, (vx_reference)in_img, obj->num_channels);

            status = vxGetStatus((vx_reference)obj->input.arr[buf]);
            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Unable to create input array! \n");
                break;
            }
            obj->input.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->input.arr[buf], 0);
        }

        vxReleaseImage(&in_img);
    }
    else
    {
        TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Unable to create input image template! \n");
    }

    return status;
}

static vx_status tiovx_multi_scaler_module_create_scaler_outputs(vx_context context, TIOVXMultiScalerModuleObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_int32 out, buf;

    if(obj->num_outputs > TIOVX_MULTI_SCALER_MODULE_MAX_OUTPUTS)
    {
        TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Number of outputs %d greater than max supported %d!\n", obj->num_outputs, TIOVX_MULTI_SCALER_MODULE_MAX_OUTPUTS);
        return VX_FAILURE;
    }

    for(out = 0; out < obj->num_outputs; out++)
    {
        if(obj->output[out].bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
        {
            TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Output buffer queue depth %d greater than max supported %d!\n", obj->output[out].bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
            return VX_FAILURE;
        }
    }

    for(out = 0; out < TIOVX_MULTI_SCALER_MODULE_MAX_OUTPUTS; out++)
    {
        for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
        {
            obj->output[out].arr[buf]  = NULL;
            obj->output[out].image_handle[buf]  = NULL;
        }
    }

    for(out = 0; out < obj->num_outputs; out++)
    {
        vx_image out_img;

        out_img = vxCreateImage(context, obj->output[out].width, obj->output[out].height, obj->color_format);
        status = vxGetStatus((vx_reference)out_img);

        if(status == VX_SUCCESS)
        {
            for(buf = 0; buf < obj->output[out].bufq_depth; buf++)
            {
                obj->output[out].arr[buf]  = vxCreateObjectArray(context, (vx_reference)out_img, obj->num_channels);

                status = vxGetStatus((vx_reference)obj->output[out].arr[buf]);
                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Unable to create output array! \n");
                    break;
                }
                else
                {
                    vx_char name[VX_MAX_REFERENCE_NAME];

                    snprintf(name, VX_MAX_REFERENCE_NAME, "scaler_node_output_arr%d_buf%d", out, buf);

                    vxSetReferenceName((vx_reference)obj->output[out].arr[buf], name);
                }

                obj->output[out].image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->output[out].arr[buf], 0);
            }
            vxReleaseImage(&out_img);
        }
        else
        {
            TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Unable to create output image template! \n");
            break;
        }
    }

    if(obj->en_multi_scalar_output == 1)
    {
        char file_path[TIVX_FILEIO_FILE_PATH_LENGTH];

        strcpy(file_path, obj->output_file_path);
        obj->file_path   = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PATH_LENGTH);
        status = vxGetStatus((vx_reference)obj->file_path);
        if(status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)obj->file_path, "scaler_write_node_file_path");

            vxAddArrayItems(obj->file_path, TIVX_FILEIO_FILE_PATH_LENGTH, &file_path[0], 1);
        }
        else
        {
            TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Unable to create file path object for fileio!\n");
        }

        for(out = 0; out < obj->num_outputs; out++)
        {
            char file_prefix[TIVX_FILEIO_FILE_PREFIX_LENGTH];

            sprintf(file_prefix, "scaler_output_%d", out);
            obj->file_prefix[out] = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PREFIX_LENGTH);
            status = vxGetStatus((vx_reference)obj->file_prefix[out]);
            if(status == VX_SUCCESS)
            {
                vx_char name[VX_MAX_REFERENCE_NAME];

                snprintf(name, VX_MAX_REFERENCE_NAME, "scaler_write_node_file_prefix_%d", out);

                vxSetReferenceName((vx_reference)obj->file_prefix[out], name);

                vxAddArrayItems(obj->file_prefix[out], TIVX_FILEIO_FILE_PREFIX_LENGTH, &file_prefix[0], 1);
            }
            else
            {
                TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Unable to create file prefix object for output %d!\n", out);
                break;
            }

            obj->write_cmd[out] = vxCreateUserDataObject(context, "tivxFileIOWriteCmd", sizeof(tivxFileIOWriteCmd), NULL);
            status = vxGetStatus((vx_reference)obj->write_cmd[out]);
            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Unable to create write cmd object for output %d!\n", out);
                break;
            }
            else
            {
                vx_char name[VX_MAX_REFERENCE_NAME];

                snprintf(name, VX_MAX_REFERENCE_NAME, "scaler_write_node_write_cmd_%d", out);

                vxSetReferenceName((vx_reference)obj->write_cmd[out], name);
            }
        }

    }
    else
    {
        obj->file_path   = NULL;
        for(out = 0; out < TIOVX_MULTI_SCALER_MODULE_MAX_OUTPUTS; out++)
        {
            obj->file_prefix[out] = NULL;
            obj->write_node[out]  = NULL;
            obj->write_cmd[out]   = NULL;
        }
    }

    return status;
}

vx_status tiovx_multi_scaler_module_init(vx_context context, TIOVXMultiScalerModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    TIOVX_MODULE_PRINTF("[MULTI-SCALER-MODULE] Configuring scaler coeffs!\n");
    status = tiovx_multi_scaler_module_configure_scaler_coeffs(context, obj);

    if((vx_status)VX_SUCCESS == status)
    {
        TIOVX_MODULE_PRINTF("[MULTI-SCALER-MODULE] Creating scaler input!\n");
        status = tiovx_multi_scaler_module_create_scaler_input(context, obj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        TIOVX_MODULE_PRINTF("[MULTI-SCALER-MODULE] Creating scaler output!\n");
        status = tiovx_multi_scaler_module_create_scaler_outputs(context, obj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        TIOVX_MODULE_PRINTF("[MULTI-SCALER-MODULE] Configuring crop params!\n");
        status = tiovx_multi_scaler_module_configure_crop_params(context, obj);
    }

    return status;
}

vx_status tiovx_multi_scaler_module_deinit(TIOVXMultiScalerModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_int32 out, buf;

    TIOVX_MODULE_PRINTF("[MULTI-SCALER-MODULE] Releasing coeffs!\n");
    status = vxReleaseUserDataObject(&obj->coeff_obj);

    for(buf = 0; buf < obj->input.bufq_depth; buf++)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[MULTI-SCALER-MODULE] Releasing input image handle, bufq %d!\n", buf);
            status = vxReleaseImage(&obj->input.image_handle[buf]);
        }
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[MULTI-SCALER-MODULE] Releasing input image arr, bufq %d!\n", buf);
            status = vxReleaseObjectArray(&obj->input.arr[buf]);
        }
    }

    for(out = 0; out < obj->num_outputs; out++)
    {
        for(buf = 0; buf < obj->output[out].bufq_depth; buf++)
        {
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[MULTI-SCALER-MODULE] Releasing output image handle, bufq = %d!\n", buf);
                status = vxReleaseImage(&obj->output[out].image_handle[buf]);
            }
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[MULTI-SCALER-MODULE] Releasing output image arr, bufq %d!\n", buf);
                status = vxReleaseObjectArray(&obj->output[out].arr[buf]);
            }
        }
        status = vxReleaseUserDataObject(obj->crop_obj + out);
    }

    if(obj->en_multi_scalar_output == 1)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[MULTI-SCALER-MODULE] Releasing output path array!\n");
            status = vxReleaseArray(&obj->file_path);
        }

        for(out = 0; out < obj->num_outputs; out++)
        {
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[MULTI-SCALER-MODULE] Releasing output %d file prefix array!\n", out);
                status = vxReleaseArray(&obj->file_prefix[out]);
            }
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[MULTI-SCALER-MODULE] Releasing output %d write command object!\n", out);
                status = vxReleaseUserDataObject(&obj->write_cmd[out]);
            }
        }
    }

    return status;
}

vx_status tiovx_multi_scaler_module_delete(TIOVXMultiScalerModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_int32 num_outputs = obj->num_outputs;
    vx_int32 out;

    if(obj->node != NULL)
    {
        TIOVX_MODULE_PRINTF("[MULTI-SCALER-MODULE] Releasing node!\n");
        status = vxReleaseNode(&obj->node);
    }
    for(out = 0; out < num_outputs; out++)
    {
        if(obj->write_node[out] != NULL)
        {
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[MULTI-SCALER-MODULE] Releasing write node [%d]!\n", out);
                status = vxReleaseNode(&obj->write_node[out]);
            }
        }
    }

    return status;
}

vx_status tiovx_multi_scaler_module_create(vx_graph graph, TIOVXMultiScalerModuleObj *obj, vx_object_array input_arr, const char* target_string)
{
    vx_status status = VX_SUCCESS;

    vx_image input;
    vx_image output1, output2, output3, output4, output5;

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

    if(obj->output[0].arr[0] != NULL)
    {
        output1 = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->output[0].arr[0], 0);
    }
    else
    {
        output1 = NULL;
    }

    if(obj->output[1].arr[0] != NULL)
    {
        output2 = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->output[1].arr[0], 0);
    }
    else
    {
        output2 = NULL;
    }

    if(obj->output[2].arr[0] != NULL)
    {
        output3 = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->output[2].arr[0], 0);
    }
    else
    {
        output3 = NULL;
    }

    if(obj->output[3].arr[0] != NULL)
    {
        output4 = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->output[3].arr[0], 0);
    }
    else
    {
        output4 = NULL;
    }

    if(obj->output[4].arr[0] != NULL)
    {
        output5 = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->output[4].arr[0], 0);
    }
    else
    {
        output5 = NULL;
    }

    obj->node = tivxVpacMscScaleNode(graph, input, output1, output2, output3, output4, output5);

    status = vxGetStatus((vx_reference)obj->node);

    if((vx_status)VX_SUCCESS == status)
    {
        vxSetNodeTarget(obj->node, VX_TARGET_STRING, target_string);
        vxSetReferenceName((vx_reference)obj->node, "scaler_node");

        vx_bool replicate[] = { vx_true_e, vx_true_e, vx_true_e, vx_true_e, vx_true_e, vx_true_e};

        if(output1 == NULL)
            replicate[1] = vx_false_e;
        if(output1 == NULL)
            replicate[2] = vx_false_e;
        if(output2 == NULL)
            replicate[3] = vx_false_e;
        if(output3 == NULL)
            replicate[4] = vx_false_e;
        if(output4 == NULL)
            replicate[5] = vx_false_e;

        vxReplicateNode(graph, obj->node, replicate, 6);

        if(obj->en_multi_scalar_output == 1)
        {
            if(output1 != NULL)
            {
                status = tiovx_multi_scaler_module_add_write_output_node(graph, obj, 0);
                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Unable to create write node for output1!\n");
                }
            }
            if(output2 != NULL)
            {
                status = tiovx_multi_scaler_module_add_write_output_node(graph, obj, 1);
                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Unable to create write node for output2!\n");
                }
            }
            if(output3 != NULL)
            {
                status = tiovx_multi_scaler_module_add_write_output_node(graph, obj, 2);
                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Unable to create write node for output3!\n");
                }
            }
            if(output4 != NULL)
            {
                status = tiovx_multi_scaler_module_add_write_output_node(graph, obj, 3);
                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Unable to create write node for output4!\n");
                }
            }
            if(output5 != NULL)
            {
                status = tiovx_multi_scaler_module_add_write_output_node(graph, obj, 4);
                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Unable to create write node for output5!\n");
                }
            }

        }
    }
    else
    {
        TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Unable to create scaler node! \n");
    }

    if(input != NULL)
        vxReleaseImage(&input);

    if(output1 != NULL)
        vxReleaseImage(&output1);
    if(output2 != NULL)
        vxReleaseImage(&output2);
    if(output3 != NULL)
        vxReleaseImage(&output3);
    if(output4 != NULL)
        vxReleaseImage(&output4);
    if(output5 != NULL)
        vxReleaseImage(&output5);

    return status;
}

vx_status tiovx_multi_scaler_module_release_buffers(TIOVXMultiScalerModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    void      *virtAddr[TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    vx_uint32   size[TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32   numEntries;
    vx_int32 out, bufq, ch;

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
                        TIOVX_MODULE_PRINTF("[MULTI-SCALER-MODULE] Freeing input, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
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
    for(out = 0; out < obj->num_outputs; out++)
    {
        for(bufq = 0; bufq < obj->output[out].bufq_depth; bufq++)
        {
            for(ch = 0; ch < obj->num_channels; ch++)
            {
                vx_reference ref = vxGetObjectArrayItem(obj->output[out].arr[bufq], ch);
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
                            TIOVX_MODULE_PRINTF("[MULTI-SCALER-MODULE] Freeing output[%d], bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", out, bufq, ch, (vx_uint64)virtAddr[0], freeSize);
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

    if ((vx_status)VX_SUCCESS != status)
    {
        TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] tivxReferenceExportHandle() failed.\n");
    }

    return status;
}

vx_status tiovx_multi_scaler_module_add_write_output_node(vx_graph graph, TIOVXMultiScalerModuleObj *obj, vx_int32 out)
{
    vx_status status = VX_SUCCESS;

    /* Need to improve this section, currently one write node can take only one image. */
    vx_image output_img = (vx_image)vxGetObjectArrayItem(obj->output[out].arr[0], 0);
    obj->write_node[out] = tivxWriteImageNode(graph, output_img, obj->file_path, obj->file_prefix[out]);
    vxReleaseImage(&output_img);

    status = vxGetStatus((vx_reference)obj->write_node[out]);

    if((vx_status)VX_SUCCESS == status)
    {
        vxSetNodeTarget(obj->write_node[out], VX_TARGET_STRING, TIVX_TARGET_A72_0);

        vx_bool replicate[] = { vx_true_e, vx_false_e, vx_false_e};
        vxReplicateNode(graph, obj->write_node[out], replicate, 3);
    }
    else
    {
        TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Unable to create fileio write node for storing outputs! \n");
    }

    return (status);
}

vx_status tiovx_multi_scaler_module_send_write_output_cmd(TIOVXMultiScalerModuleObj *obj, vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip)
{
    vx_status status = VX_SUCCESS;

    tivxFileIOWriteCmd write_cmd;
    vx_int32 out;

    write_cmd.start_frame = start_frame;
    write_cmd.num_frames = num_frames;
    write_cmd.num_skip = num_skip;

    for(out = 0; out < obj->num_outputs; out++)
    {
        status = vxCopyUserDataObject(obj->write_cmd[out], 0, sizeof(tivxFileIOWriteCmd),\
                  &write_cmd, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

        if((vx_status)VX_SUCCESS == status)
        {
            vx_reference refs[2];

            refs[0] = (vx_reference)obj->write_cmd[out];

            status = tivxNodeSendCommand(obj->write_node[out], TIVX_CONTROL_CMD_SEND_TO_ALL_REPLICATED_NODES,
                                    TIVX_FILEIO_CMD_SET_FILE_WRITE,
                                    refs, 1u);

            if(VX_SUCCESS != status)
            {
                TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] write node send command failed!\n");
            }

            TIOVX_MODULE_PRINTF("[MULTI-SCALER-MODULE] write node send command success!\n");
        }
    }

    return (status);
}

void tiovx_multi_scaler_module_set_coeff(tivx_vpac_msc_coefficients_t *coeff, uint32_t interpolation)
{
    uint32_t i;
    uint32_t idx;
    uint32_t weight;

    idx = 0;
    coeff->single_phase[0][idx ++] = 0;
    coeff->single_phase[0][idx ++] = 0;
    coeff->single_phase[0][idx ++] = 256;
    coeff->single_phase[0][idx ++] = 0;
    coeff->single_phase[0][idx ++] = 0;
    idx = 0;
    coeff->single_phase[1][idx ++] = 0;
    coeff->single_phase[1][idx ++] = 0;
    coeff->single_phase[1][idx ++] = 256;
    coeff->single_phase[1][idx ++] = 0;
    coeff->single_phase[1][idx ++] = 0;

    if (VX_INTERPOLATION_BILINEAR == interpolation)
    {
        idx = 0;
        for(i=0; i<32; i++)
        {
            weight = i<<2;
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 256-weight;
            coeff->multi_phase[0][idx ++] = weight;
            coeff->multi_phase[0][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            weight = (i+32)<<2;
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 256-weight;
            coeff->multi_phase[1][idx ++] = weight;
            coeff->multi_phase[1][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            weight = i<<2;
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 256-weight;
            coeff->multi_phase[2][idx ++] = weight;
            coeff->multi_phase[2][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            weight = (i+32)<<2;
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 256-weight;
            coeff->multi_phase[3][idx ++] = weight;
            coeff->multi_phase[3][idx ++] = 0;
        }
    }
    else /* STR_VX_INTERPOLATION_NEAREST_NEIGHBOR */
    {
        idx = 0;
        for(i=0; i<32; i++)
        {
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 256;
            coeff->multi_phase[0][idx ++] = 0;
            coeff->multi_phase[0][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 0;
            coeff->multi_phase[1][idx ++] = 256;
            coeff->multi_phase[1][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 256;
            coeff->multi_phase[2][idx ++] = 0;
            coeff->multi_phase[2][idx ++] = 0;
        }
        idx = 0;
        for(i=0; i<32; i++)
        {
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 0;
            coeff->multi_phase[3][idx ++] = 256;
            coeff->multi_phase[3][idx ++] = 0;
        }
    }
}

vx_status tiovx_multi_scaler_module_update_filter_coeffs(TIOVXMultiScalerModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_reference refs[1];

    refs[0] = (vx_reference)obj->coeff_obj;
    if((vx_status)VX_SUCCESS == status)
    {
      status = tivxNodeSendCommand(obj->node, 0u,
                                  TIVX_VPAC_MSC_CMD_SET_COEFF,
                                  refs, 1u);

      TIOVX_MODULE_PRINTF("[MULTI-SCALER-MODULE] App Send MSC Command Done!\n");
    }

    if((vx_status)VX_SUCCESS != status)
    {
        TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Node send command failed!\n");
    }

    return status;
}

void tiovx_multi_scaler_module_crop_params_init(TIOVXMultiScalerModuleObj *obj)
{
    vx_int32 out;

    for (out = 0; out < obj->num_outputs; out++)
    {
        obj->crop_params[out].crop_start_x = 0;
        obj->crop_params[out].crop_start_y = 0;
        obj->crop_params[out].crop_width = obj->input.width;
        obj->crop_params[out].crop_height = obj->input.height;
    }
}

vx_status tiovx_multi_scaler_module_update_crop_params(TIOVXMultiScalerModuleObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_reference refs[TIOVX_MULTI_SCALER_MODULE_MAX_OUTPUTS];
    vx_int32 out;

    for (out = 0; out < obj->num_outputs; out++)
    {
        refs[out] = (vx_reference)obj->crop_obj[out];
    }

    status = tivxNodeSendCommand(obj->node, 0u,
            TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS,
            refs, obj->num_outputs);

    if((vx_status)VX_SUCCESS != status)
    {
        TIOVX_MODULE_ERROR("[MULTI-SCALER-MODULE] Node send command TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS, failed!\n");
    }

    return status;
}
