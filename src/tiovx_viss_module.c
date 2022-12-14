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

#include "tiovx_viss_module.h"

static vx_status tiovx_viss_module_configure_params(vx_context context, TIOVXVISSModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    SensorObj *sensorObj = obj->sensorObj;

    obj->params.sensor_dcc_id       = sensorObj->sensorParams.dccId;
    obj->params.use_case            = 0;
    obj->params.fcp[0].ee_mode      = TIVX_VPAC_VISS_EE_MODE_OFF;
    obj->params.fcp[0].chroma_mode  = TIVX_VPAC_VISS_CHROMA_MODE_420;

    if(obj->output_select[0] == TIOVX_VISS_MODULE_OUTPUT_EN)
    {
#if defined(SOC_AM62A)
        if(obj->params.enable_ir_op)
        {
            if(obj->output0.color_format == VX_DF_IMAGE_U8)
            {
                obj->params.fcp[0].mux_output0  = TIVX_VPAC_VISS_MUX0_IR8;
                /* If IR output is 8 bit use TIVX_VPAC_VISS_MUX0_IR8 
                If IR output is Packed 12 bit use TIVX_VPAC_VISS_MUX0_IR12_P12*/
            }
            else if(obj->output0.color_format == TIVX_DF_IMAGE_P12)
            {
                obj->params.fcp[0].mux_output0  = TIVX_VPAC_VISS_MUX0_IR12_P12;
            }
        }
        else
#endif
        {
            obj->params.fcp[0].mux_output0  = 0;
        }
    }
    if(obj->output_select[1] == TIOVX_VISS_MODULE_OUTPUT_EN)
    {
        obj->params.fcp[0].mux_output1  = 0;
    }
    if(obj->output_select[2] == TIOVX_VISS_MODULE_OUTPUT_EN)
    {
        if(obj->output2.color_format == VX_DF_IMAGE_NV12)
        {
            obj->params.fcp[0].mux_output2  = TIVX_VPAC_VISS_MUX2_NV12;
        }
        else if((obj->output2.color_format == VX_DF_IMAGE_YUYV) ||
                (obj->output2.color_format == VX_DF_IMAGE_UYVY))
        {
            obj->params.fcp[0].mux_output2  = TIVX_VPAC_VISS_MUX2_YUV422;
        }
#if defined(SOC_AM62A)
        else if((obj->output2.color_format == VX_DF_IMAGE_U16) &&
                (obj->params.enable_ir_op))
        {
            obj->params.fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_IR12_U16;
        }
#endif
    }
    if(obj->output_select[3] == TIOVX_VISS_MODULE_OUTPUT_EN)
    {
        obj->params.fcp[0].mux_output3  = 0;
    }
    if(obj->output_select[4] == TIOVX_VISS_MODULE_OUTPUT_EN)
    {
        obj->params.fcp[0].mux_output4  = 0;
    }

#if defined(SOC_AM62A)
    if(obj->params.bypass_pcid)
        obj->params.enable_ir_op = TIVX_VPAC_VISS_IR_DISABLE;

    if(obj->params.enable_ir_op)
        obj->params.h3a_in              = TIVX_VPAC_VISS_H3A_IN_LSC;
    else if(obj->params.enable_bayer_op && !(obj->params.bypass_pcid))
        obj->params.h3a_in              = TIVX_VPAC_VISS_H3A_IN_PCID;
    else if(obj->params.enable_bayer_op && obj->params.bypass_pcid)
        obj->params.h3a_in              = TIVX_VPAC_VISS_H3A_IN_LSC;
#else
    obj->params.h3a_in              = TIVX_VPAC_VISS_H3A_IN_LSC;  
#endif
    obj->params.h3a_aewb_af_mode    = TIVX_VPAC_VISS_H3A_MODE_AEWB;
    obj->params.bypass_nsf4         = 0;
    obj->params.enable_ctx          = 1;

    if(sensorObj->sensor_wdr_enabled == 1)
    {
        obj->params.bypass_glbce = 0;
    }
    else
    {
        obj->params.bypass_glbce = 1;
    }

    obj->config = vxCreateUserDataObject(context, "tivx_vpac_viss_params_t", sizeof(tivx_vpac_viss_params_t), &obj->params);
    status = vxGetStatus((vx_reference)obj->config);

    if(status != VX_SUCCESS)
    {
        TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create VISS config object! \n");
    }

    return status;
}

static vx_status tiovx_viss_module_configure_dcc_params(vx_context context, TIOVXVISSModuleObj *obj)
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

            obj->dcc_config = vxCreateUserDataObject(context, "dcc_viss", dcc_buff_size, NULL );
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
                    TIOVX_MODULE_ERROR("[VISS-MODULE] DCC config bytes read %d not matching bytes expected %d \n", bytes_read, dcc_buff_size);
                    status = VX_FAILURE;
                }

                vxUnmapUserDataObject(obj->dcc_config, dcc_buf_map_id);
            }
            else
            {
                TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create DCC config object! \n");
            }
        }
    }

    return status;
}

static vx_status tiovx_viss_module_create_inputs(vx_context context, TIOVXVISSModuleObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_int32 buf;

    SensorObj *sensorObj = obj->sensorObj;

    /* Create ae_awb results buffer (uninitialized) */
    if(obj->ae_awb_result_bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[VISS-MODULE] ae-awb result buffer queue depth %d greater than max supported %d!\n", obj->ae_awb_result_bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
        return VX_FAILURE;
    }

    for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
    {
        obj->ae_awb_result_arr[buf]  = NULL;
        obj->ae_awb_result_handle[buf]  = NULL;
    }

    vx_user_data_object ae_awb_result = vxCreateUserDataObject(context, "tivx_ae_awb_params_t", sizeof(tivx_ae_awb_params_t), NULL);
    status = vxGetStatus((vx_reference)ae_awb_result);

    if((vx_status)VX_SUCCESS == status)
    {
        for(buf = 0; buf < obj->ae_awb_result_bufq_depth; buf++)
        {
            obj->ae_awb_result_arr[buf] = vxCreateObjectArray(context, (vx_reference)ae_awb_result, sensorObj->num_cameras_enabled);
            status = vxGetStatus((vx_reference)obj->ae_awb_result_arr[buf]);

            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create ae-awb result object array! \n");
                break;
            }
            obj->ae_awb_result_handle[buf] = (vx_user_data_object)vxGetObjectArrayItem((vx_object_array)obj->ae_awb_result_arr[buf], 0);
        }
        vxReleaseUserDataObject(&ae_awb_result);
    }
    else
    {
        TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create ae-awb result object! \n");
    }

    if((vx_status)VX_SUCCESS == status)
    {
        if(obj->input.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
        {
            TIOVX_MODULE_ERROR("[VISS-MODULE] input raw image buffer queue depth %d greater than max supported %d!\n", obj->input.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
            return VX_FAILURE;
        }

        for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
        {
            obj->input.arr[buf]  = NULL;
            obj->input.image_handle[buf]  = NULL;
        }

        tivx_raw_image raw_image = tivxCreateRawImage(context, &obj->input.params);
        status = vxGetStatus((vx_reference)raw_image);

        if((vx_status)VX_SUCCESS == status)
        {
            for(buf = 0; buf < obj->input.bufq_depth; buf++)
            {
                obj->input.arr[buf] = vxCreateObjectArray(context, (vx_reference)raw_image, sensorObj->num_cameras_enabled);
                status = vxGetStatus((vx_reference)obj->input.arr[buf]);

                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create input raw image array! \n");
                }
                obj->input.image_handle[buf] = (tivx_raw_image)vxGetObjectArrayItem((vx_object_array)obj->input.arr[buf], 0);
            }
            tivxReleaseRawImage(&raw_image);
        }
        else
        {
            TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create input raw image! \n");
        }
    }

    return status;
}

static vx_status tiovx_viss_module_create_outputs(vx_context context, TIOVXVISSModuleObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_int32 buf;

    SensorObj *sensorObj = obj->sensorObj;

    /* Create h3a_aew_af output buffer (uninitialized) */
    if(obj->h3a_stats_bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
    {
        TIOVX_MODULE_ERROR("[VISS-MODULE] h3a stats buffer queue depth %d greater than max supported %d!\n", obj->h3a_stats_bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
        return VX_FAILURE;
    }

    for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
    {
        obj->h3a_stats_arr[buf]  = NULL;
        obj->h3a_stats_handle[buf]  = NULL;
    }

    vx_user_data_object h3a_stats = vxCreateUserDataObject(context, "tivx_h3a_data_t", sizeof(tivx_h3a_data_t), NULL);
    status = vxGetStatus((vx_reference)h3a_stats);

    if((vx_status)VX_SUCCESS == status)
    {
        for(buf = 0; buf < obj->h3a_stats_bufq_depth; buf++)
        {
            obj->h3a_stats_arr[buf] = vxCreateObjectArray(context, (vx_reference)h3a_stats, sensorObj->num_cameras_enabled);
            status = vxGetStatus((vx_reference)obj->h3a_stats_arr[buf]);

            if(status != VX_SUCCESS)
            {
                TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create h3a stats object array! \n");
                break;
            }
            obj->h3a_stats_handle[buf] = (vx_user_data_object)vxGetObjectArrayItem((vx_object_array)obj->h3a_stats_arr[buf], 0);
        }
        vxReleaseUserDataObject(&h3a_stats);
    }
    else
    {
        TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create h3a stats object! \n");
    }

    /* Create output0 buffer */
    if(((vx_status)VX_SUCCESS == status) && (obj->output_select[0] == TIOVX_VISS_MODULE_OUTPUT_EN))
    {
        if(obj->output0.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
        {
            TIOVX_MODULE_ERROR("[VISS-MODULE] output0 buffer queue depth %d greater than max supported %d!\n", obj->output0.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
            return VX_FAILURE;
        }

        for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
        {
            obj->output0.arr[buf]  = NULL;
            obj->output0.image_handle[buf]  = NULL;
        }

        vx_image output_img = vxCreateImage(context, obj->output0.width, obj->output0.height, obj->output0.color_format);
        status = vxGetStatus((vx_reference)output_img);

        if((vx_status)VX_SUCCESS == status)
        {
            for(buf = 0; buf < obj->output0.bufq_depth; buf++)
            {
                obj->output0.arr[buf] = vxCreateObjectArray(context, (vx_reference)output_img, sensorObj->num_cameras_enabled);
                status = vxGetStatus((vx_reference)obj->output0.arr[buf]);

                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create VISS output0 image array! \n");
                }
                obj->output0.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->output0.arr[buf], 0);
            }

            vxReleaseImage(&output_img);
        }
        else
        {
            TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create VISS output0 image! \n");
        }
    }

    /* Create output1 buffer */
    if(((vx_status)VX_SUCCESS == status) && (obj->output_select[1] == TIOVX_VISS_MODULE_OUTPUT_EN))
    {
        if(obj->output1.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
        {
            TIOVX_MODULE_ERROR("[VISS-MODULE] output1 buffer queue depth %d greater than max supported %d!\n", obj->output1.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
            return VX_FAILURE;
        }

        for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
        {
            obj->output1.arr[buf]  = NULL;
            obj->output1.image_handle[buf]  = NULL;
        }

        vx_image output_img = vxCreateImage(context, obj->output1.width, obj->output1.height, obj->output1.color_format);
        status = vxGetStatus((vx_reference)output_img);

        if((vx_status)VX_SUCCESS == status)
        {
            for(buf = 0; buf < obj->output1.bufq_depth; buf++)
            {
                obj->output1.arr[buf] = vxCreateObjectArray(context, (vx_reference)output_img, sensorObj->num_cameras_enabled);
                status = vxGetStatus((vx_reference)obj->output1.arr[buf]);

                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create VISS output1 image array! \n");
                }
                obj->output1.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->output1.arr[buf], 0);
            }

            vxReleaseImage(&output_img);
        }
        else
        {
            TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create VISS output1 image! \n");
        }
    }

    /* Create output2 buffer */
    if(((vx_status)VX_SUCCESS == status) && (obj->output_select[2] == TIOVX_VISS_MODULE_OUTPUT_EN))
    {
        if(obj->output2.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
        {
            TIOVX_MODULE_ERROR("[VISS-MODULE] output2 buffer queue depth %d greater than max supported %d!\n", obj->output2.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
            return VX_FAILURE;
        }

        for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
        {
            obj->output2.arr[buf]  = NULL;
            obj->output2.image_handle[buf]  = NULL;
        }

        vx_image output_img = vxCreateImage(context, obj->output2.width, obj->output2.height, obj->output2.color_format);
        status = vxGetStatus((vx_reference)output_img);

        if((vx_status)VX_SUCCESS == status)
        {
            for(buf = 0; buf < obj->output2.bufq_depth; buf++)
            {
                obj->output2.arr[buf] = vxCreateObjectArray(context, (vx_reference)output_img, sensorObj->num_cameras_enabled);
                status = vxGetStatus((vx_reference)obj->output2.arr[buf]);

                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create VISS output2 image array! \n");
                }
                obj->output2.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->output2.arr[buf], 0);
            }

            vxReleaseImage(&output_img);
        }
        else
        {
            TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create VISS output2 image! \n");
        }
    }

    /* Create output3 buffer */
    if(((vx_status)VX_SUCCESS == status) && (obj->output_select[3] == TIOVX_VISS_MODULE_OUTPUT_EN))
    {
        if(obj->output3.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
        {
            TIOVX_MODULE_ERROR("[VISS-MODULE] output3 buffer queue depth %d greater than max supported %d!\n", obj->output3.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
            return VX_FAILURE;
        }

        for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
        {
            obj->output3.arr[buf]  = NULL;
            obj->output3.image_handle[buf]  = NULL;
        }

        vx_image output_img = vxCreateImage(context, obj->output3.width, obj->output3.height, obj->output3.color_format);
        status = vxGetStatus((vx_reference)output_img);

        if((vx_status)VX_SUCCESS == status)
        {
            for(buf = 0; buf < obj->output3.bufq_depth; buf++)
            {
                obj->output3.arr[buf] = vxCreateObjectArray(context, (vx_reference)output_img, sensorObj->num_cameras_enabled);
                status = vxGetStatus((vx_reference)obj->output3.arr[buf]);

                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create VISS output3 image array! \n");
                }
                obj->output3.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->output3.arr[buf], 0);
            }

            vxReleaseImage(&output_img);
        }
        else
        {
            TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create VISS output3 image! \n");
        }
    }

    /* Create output4 buffer */
    if(((vx_status)VX_SUCCESS == status) && (obj->output_select[4] == TIOVX_VISS_MODULE_OUTPUT_EN))
    {
        if(obj->output4.bufq_depth > TIOVX_MODULES_MAX_BUFQ_DEPTH)
        {
            TIOVX_MODULE_ERROR("[VISS-MODULE] output4 buffer queue depth %d greater than max supported %d!\n", obj->output4.bufq_depth, TIOVX_MODULES_MAX_BUFQ_DEPTH);
            return VX_FAILURE;
        }

        for(buf = 0; buf < TIOVX_MODULES_MAX_BUFQ_DEPTH; buf++)
        {
            obj->output4.arr[buf]  = NULL;
            obj->output4.image_handle[buf]  = NULL;
        }

        vx_image output_img = vxCreateImage(context, obj->output4.width, obj->output4.height, obj->output4.color_format);
        status = vxGetStatus((vx_reference)output_img);

        if((vx_status)VX_SUCCESS == status)
        {
            for(buf = 0; buf < obj->output4.bufq_depth; buf++)
            {
                obj->output4.arr[buf] = vxCreateObjectArray(context, (vx_reference)output_img, sensorObj->num_cameras_enabled);
                status = vxGetStatus((vx_reference)obj->output4.arr[buf]);

                if(status != VX_SUCCESS)
                {
                    TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create VISS output4 image array! \n");
                }
                obj->output4.image_handle[buf] = (vx_image)vxGetObjectArrayItem((vx_object_array)obj->output4.arr[buf], 0);
            }

            vxReleaseImage(&output_img);
        }
        else
        {
            TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create VISS output4 image! \n");
        }
    }

    if(obj->en_out_viss_write == 1)
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
            TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create file path object for storing VISS outputs! \n");
        }


        strcpy(file_prefix, "viss_img_output");
        obj->img_file_prefix = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PREFIX_LENGTH);
        status = vxGetStatus((vx_reference)obj->img_file_prefix);
        if((vx_status)VX_SUCCESS == status)
        {
            vxAddArrayItems(obj->img_file_prefix, TIVX_FILEIO_FILE_PREFIX_LENGTH, &file_prefix[0], 1);
        }
        else
        {
            TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create file prefix object for storing VISS output image! \n");
        }

        strcpy(file_prefix, "viss_h3a_output");
        obj->h3a_file_prefix = vxCreateArray(context, VX_TYPE_UINT8, TIVX_FILEIO_FILE_PREFIX_LENGTH);
        status = vxGetStatus((vx_reference)obj->h3a_file_prefix);
        if((vx_status)VX_SUCCESS == status)
        {
            vxAddArrayItems(obj->h3a_file_prefix, TIVX_FILEIO_FILE_PREFIX_LENGTH, &file_prefix[0], 1);
        }
        else
        {
            TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create file prefix object for storing VISS h3a stats! \n");
        }

        obj->write_cmd = vxCreateUserDataObject(context, "tivxFileIOWriteCmd", sizeof(tivxFileIOWriteCmd), NULL);
        status = vxGetStatus((vx_reference)obj->write_cmd);
        if(status != VX_SUCCESS)
        {
            TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create fileio write cmd object for VISS node! \n");
        }
    }
    else
    {
        obj->file_path        = NULL;
        obj->img_file_prefix  = NULL;
        obj->h3a_file_prefix  = NULL;
        obj->img_write_node   = NULL;
        obj->h3a_write_node   = NULL;
        obj->write_cmd        = NULL;
    }

    return status;
}

vx_status tiovx_viss_module_init(vx_context context, TIOVXVISSModuleObj *obj, SensorObj *sensorObj)
{
    vx_status status = VX_SUCCESS;

    obj->sensorObj = sensorObj;

    if(obj->sensorObj == NULL)
    {
        TIOVX_MODULE_ERROR("Sensor Object handle is NULL!");
        status = VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_viss_module_configure_params(context, obj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_viss_module_configure_dcc_params(context, obj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_viss_module_create_inputs(context, obj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_viss_module_create_outputs(context, obj);
    }

#if defined(ENABLE_DCC_TOOL)
    if((vx_status)VX_SUCCESS == status)
    {
        status = itt_register_object(context,
                                     &(obj->node),
                                     &(obj->input.image_handle[0]),
                                     &(obj->output2.image_handle[0]),
                                     VISS);
    }
#endif

    return (status);
}

vx_status tiovx_viss_module_deinit(TIOVXVISSModuleObj *obj)
{
    vx_status status = VX_SUCCESS;
    vx_int32 buf;

    if(((vx_status)VX_SUCCESS == status) && (obj->config != NULL))
    {
        TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing VISS config handle!\n");
        status = vxReleaseUserDataObject(&obj->config);
    }

    if(((vx_status)VX_SUCCESS == status) && (obj->dcc_config != NULL))
    {
        TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing DCC config handle!\n");
        status = vxReleaseUserDataObject(&obj->dcc_config);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        for(buf = 0; buf < obj->ae_awb_result_bufq_depth; buf++)
        {
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing ae-awb result handle!\n");
                status = vxReleaseUserDataObject(&obj->ae_awb_result_handle[buf]);
            }
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing ae-awb result arr!\n");
                status = vxReleaseObjectArray(&obj->ae_awb_result_arr[buf]);
            }
        }
    }

    if((vx_status)VX_SUCCESS == status)
    {
        for(buf = 0; buf < obj->input.bufq_depth; buf++)
        {
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing raw input image handle!\n");
                status = tivxReleaseRawImage(&obj->input.image_handle[buf]);
            }
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing raw input image arr!\n");
                status = vxReleaseObjectArray(&obj->input.arr[buf]);
            }
        }
    }

    if((vx_status)VX_SUCCESS == status)
    {
        for(buf = 0; buf < obj->h3a_stats_bufq_depth; buf++)
        {
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing h3a stats handle!\n");
                status = vxReleaseUserDataObject(&obj->h3a_stats_handle[buf]);
            }
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing h3a stats arr!\n");
                status = vxReleaseObjectArray(&obj->h3a_stats_arr[buf]);
            }
        }
    }

    if(((vx_status)VX_SUCCESS == status) && (obj->output_select[0] == TIOVX_VISS_MODULE_OUTPUT_EN))
    {
        for(buf = 0; buf < obj->output0.bufq_depth; buf++)
        {
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing output0 image handle!\n");
                status = vxReleaseImage(&obj->output0.image_handle[buf]);
            }
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing output0 image arr!\n");
                status = vxReleaseObjectArray(&obj->output0.arr[buf]);
            }
        }
    }

    if(((vx_status)VX_SUCCESS == status) && (obj->output_select[1] == TIOVX_VISS_MODULE_OUTPUT_EN))
    {
        for(buf = 0; buf < obj->output1.bufq_depth; buf++)
        {
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing output1 image handle!\n");
                status = vxReleaseImage(&obj->output1.image_handle[buf]);
            }
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing output1 image arr!\n");
                status = vxReleaseObjectArray(&obj->output1.arr[buf]);
            }
        }
    }

    if(((vx_status)VX_SUCCESS == status) && (obj->output_select[2] == TIOVX_VISS_MODULE_OUTPUT_EN))
    {
        for(buf = 0; buf < obj->output2.bufq_depth; buf++)
        {
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing output2 image handle!\n");
                status = vxReleaseImage(&obj->output2.image_handle[buf]);
            }
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing output2 image arr!\n");
                status = vxReleaseObjectArray(&obj->output2.arr[buf]);
            }
        }
    }

    if(((vx_status)VX_SUCCESS == status) && (obj->output_select[3] == TIOVX_VISS_MODULE_OUTPUT_EN))
    {
        for(buf = 0; buf < obj->output3.bufq_depth; buf++)
        {
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing output3 image handle!\n");
                status = vxReleaseImage(&obj->output3.image_handle[buf]);
            }
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing output3 image arr!\n");
                status = vxReleaseObjectArray(&obj->output3.arr[buf]);
            }
        }
    }

    if(((vx_status)VX_SUCCESS == status) && (obj->output_select[4] == TIOVX_VISS_MODULE_OUTPUT_EN))
    {
        for(buf = 0; buf < obj->output4.bufq_depth; buf++)
        {
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing output4 image handle!\n");
                status = vxReleaseImage(&obj->output4.image_handle[buf]);
            }
            if((vx_status)VX_SUCCESS == status)
            {
                TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing output4 image arr!\n");
                status = vxReleaseObjectArray(&obj->output4.arr[buf]);
            }
        }
    }

    if(obj->en_out_viss_write == 1)
    {
        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing output path array!\n");
            status = vxReleaseArray(&obj->file_path);
        }

        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing output image file prefix array!\n");
            status = vxReleaseArray(&obj->img_file_prefix);
        }

        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing h3a stats file prefix array!\n");
            status = vxReleaseArray(&obj->h3a_file_prefix);
        }

        if((vx_status)VX_SUCCESS == status)
        {
            TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing output write command object!\n");
            status = vxReleaseUserDataObject(&obj->write_cmd);
        }
    }

    return status;
}

vx_status tiovx_viss_module_delete(TIOVXVISSModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    if(obj->node != NULL)
    {
        TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing node reference!\n");
        status = vxReleaseNode(&obj->node);
    }

    if(((vx_status)VX_SUCCESS == status) && (obj->img_write_node != NULL))
    {
        TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing image write node reference!\n");
        status = vxReleaseNode(&obj->img_write_node);
    }

    if(((vx_status)VX_SUCCESS == status) && (obj->h3a_write_node != NULL))
    {
        TIOVX_MODULE_PRINTF("[VISS-MODULE] Releasing h3a write node reference!\n");
        status = vxReleaseNode(&obj->h3a_write_node);
    }

    return status;
}

vx_status tiovx_viss_module_create(vx_graph graph, TIOVXVISSModuleObj *obj, vx_object_array raw_image_arr, vx_object_array ae_awb_result_arr, const char* target_string)
{
    vx_status status = VX_SUCCESS;

    tivx_raw_image raw_image = NULL;
    vx_user_data_object ae_awb_result = NULL;
    vx_image output0 = NULL;
    vx_image output1 = NULL;
    vx_image output2 = NULL;
    vx_image output3 = NULL;
    vx_image output4 = NULL;
    vx_user_data_object h3a_stats = NULL;

    if(raw_image_arr != NULL)
    {
        raw_image = (tivx_raw_image)vxGetObjectArrayItem(raw_image_arr, 0);
    }
    else
    {
        raw_image = (tivx_raw_image)vxGetObjectArrayItem(obj->input.arr[0], 0);
    }

    if(ae_awb_result_arr != NULL)
    {
        ae_awb_result = (vx_user_data_object)vxGetObjectArrayItem(ae_awb_result_arr, 0);
    }
    else
    {
        ae_awb_result = (vx_user_data_object)vxGetObjectArrayItem(obj->ae_awb_result_arr[0], 0);
    }

    h3a_stats = (vx_user_data_object)vxGetObjectArrayItem(obj->h3a_stats_arr[0], 0);

    if(obj->output_select[0] == TIOVX_VISS_MODULE_OUTPUT_EN)
    {
        output0 = (vx_image)vxGetObjectArrayItem(obj->output0.arr[0], 0);
    }
    if(obj->output_select[1] == TIOVX_VISS_MODULE_OUTPUT_EN)
    {
        output1 = (vx_image)vxGetObjectArrayItem(obj->output1.arr[0], 0);
    }
    if(obj->output_select[2] == TIOVX_VISS_MODULE_OUTPUT_EN)
    {
        output2 = (vx_image)vxGetObjectArrayItem(obj->output2.arr[0], 0);
    }
    if(obj->output_select[3] == TIOVX_VISS_MODULE_OUTPUT_EN)
    {
        output3 = (vx_image)vxGetObjectArrayItem(obj->output3.arr[0], 0);
    }
    if(obj->output_select[4] == TIOVX_VISS_MODULE_OUTPUT_EN)
    {
        output4 = (vx_image)vxGetObjectArrayItem(obj->output4.arr[0], 0);
    }

    obj->node = tivxVpacVissNode(graph,
                                obj->config,
                                ae_awb_result,
                                obj->dcc_config,
                                raw_image,
                                output0,
                                output1,
                                output2,
                                output3,
                                output4,
                                h3a_stats, NULL, NULL, NULL);

    status = vxGetStatus((vx_reference)obj->node);
    if((vx_status)VX_SUCCESS == status)
    {
        vxSetNodeTarget(obj->node, VX_TARGET_STRING, target_string);

        vx_bool replicate[13];

        replicate[0] = vx_false_e;
        replicate[1] = vx_false_e;
        replicate[2] = vx_false_e;
        replicate[3] = vx_true_e;

        if(obj->output_select[0] == TIOVX_VISS_MODULE_OUTPUT_EN)
            replicate[4] = vx_true_e;
        else
            replicate[4] = vx_false_e;

        if(obj->output_select[1] == TIOVX_VISS_MODULE_OUTPUT_EN)
            replicate[5] = vx_true_e;
        else
            replicate[5] = vx_false_e;

        if(obj->output_select[2] == TIOVX_VISS_MODULE_OUTPUT_EN)
            replicate[6] = vx_true_e;
        else
            replicate[6] = vx_false_e;

        if(obj->output_select[3] == TIOVX_VISS_MODULE_OUTPUT_EN)
            replicate[7] = vx_true_e;
        else
            replicate[7] = vx_false_e;

        if(obj->output_select[4] == TIOVX_VISS_MODULE_OUTPUT_EN)
            replicate[8] = vx_true_e;
        else
            replicate[8] = vx_false_e;

        replicate[9] = vx_true_e;
        replicate[10] = vx_false_e;
        replicate[11] = vx_false_e;
        replicate[12] = vx_false_e;

        vxReplicateNode(graph, obj->node, replicate, 13);

    }
    else
    {
        TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create VISS Node! \n");
    }

    tivxReleaseRawImage(&raw_image);
    vxReleaseUserDataObject(&ae_awb_result);
    vxReleaseUserDataObject(&h3a_stats);

    if(obj->output_select[0] == TIOVX_VISS_MODULE_OUTPUT_EN)
    {
        vxReleaseImage(&output0);
    }
    if(obj->output_select[1] == TIOVX_VISS_MODULE_OUTPUT_EN)
    {
        vxReleaseImage(&output1);
    }
    if(obj->output_select[2] == TIOVX_VISS_MODULE_OUTPUT_EN)
    {
        vxReleaseImage(&output2);
    }
    if(obj->output_select[3] == TIOVX_VISS_MODULE_OUTPUT_EN)
    {
        vxReleaseImage(&output3);
    }
    if(obj->output_select[4] == TIOVX_VISS_MODULE_OUTPUT_EN)
    {
        vxReleaseImage(&output4);
    }

    if(obj->en_out_viss_write == 1)
    {
        status = tiovx_viss_module_add_write_output_node(graph, obj);
    }

#if defined(ENABLE_DCC_TOOL)
    status = itt_server_edge_ai_init();
#endif
    
    return status;
}

vx_status tiovx_viss_module_release_buffers(TIOVXVISSModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    SensorObj *sensorObj = obj->sensorObj;

    void       *virtAddr[TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    vx_uint32   size[TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32   numEntries;
    vx_int32    bufq, ch;

    /* Free raw input handles */
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

                    for(ctr = 0; ctr < numEntries; ctr++)
                    {
                        if(virtAddr[ctr] != NULL)
                        {
                            TIOVX_MODULE_PRINTF("[VISS-MODULE] Freeing raw input, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[ctr], size[ctr]);
                            tivxMemFree(virtAddr[ctr], size[ctr], TIVX_MEM_EXTERNAL);
                        }
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

    /* Free ae_awb_result input handles */
    for(bufq = 0; bufq < obj->ae_awb_result_bufq_depth; bufq++)
    {
        for(ch = 0; ch < sensorObj->num_cameras_enabled; ch++)
        {
            vx_reference ref = vxGetObjectArrayItem(obj->ae_awb_result_arr[bufq], ch);
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

                    for(ctr = 0; ctr < numEntries; ctr++)
                    {
                        if(virtAddr[ctr] != NULL)
                        {
                            TIOVX_MODULE_PRINTF("[VISS-MODULE] Freeing ae-awb result, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[ctr], size[ctr]);
                            tivxMemFree(virtAddr[ctr], size[ctr], TIVX_MEM_EXTERNAL);
                        }
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

    /* Free h3a_stats output handles */
    for(bufq = 0; bufq < obj->h3a_stats_bufq_depth; bufq++)
    {
        for(ch = 0; ch < sensorObj->num_cameras_enabled; ch++)
        {
            vx_reference ref = vxGetObjectArrayItem(obj->h3a_stats_arr[bufq], ch);
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

                    for(ctr = 0; ctr < numEntries; ctr++)
                    {
                        if(virtAddr[ctr] != NULL)
                        {
                            TIOVX_MODULE_PRINTF("[VISS-MODULE] Freeing h3a stats, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[ctr], size[ctr]);
                            tivxMemFree(virtAddr[ctr], size[ctr], TIVX_MEM_EXTERNAL);
                        }
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

    /* Free output0 handles */
    if(obj->output_select[0] == TIOVX_VISS_MODULE_OUTPUT_EN)
    {
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
                        /* Currently the vx_image buffers are allocated in one shot for multiple planes.
                            So if we are freeing this buffer then we need to get only the first plane
                            pointer address but add up the all the sizes to free the entire buffer */
                        vx_uint32 freeSize = 0;
                        for(ctr = 0; ctr < numEntries; ctr++)
                        {
                            freeSize += size[ctr];
                        }

                        if(virtAddr[0] != NULL)
                        {
                            TIOVX_MODULE_PRINTF("[VISS-MODULE] Freeing output0, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
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

    /* Free output1 handles */
    if(obj->output_select[1] == TIOVX_VISS_MODULE_OUTPUT_EN)
    {
        for(bufq = 0; bufq < obj->output1.bufq_depth; bufq++)
        {
            for(ch = 0; ch < sensorObj->num_cameras_enabled; ch++)
            {
                vx_reference ref = vxGetObjectArrayItem(obj->output1.arr[bufq], ch);
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
                        /* Currently the vx_image buffers are allocated in one shot for multiple planes.
                            So if we are freeing this buffer then we need to get only the first plane
                            pointer address but add up the all the sizes to free the entire buffer */
                        vx_uint32 freeSize = 0;
                        for(ctr = 0; ctr < numEntries; ctr++)
                        {
                            freeSize += size[ctr];
                        }

                        if(virtAddr[0] != NULL)
                        {
                            TIOVX_MODULE_PRINTF("[VISS-MODULE] Freeing output1, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
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

    /* Free output2 handles */
    if(obj->output_select[2] == TIOVX_VISS_MODULE_OUTPUT_EN)
    {
        for(bufq = 0; bufq < obj->output2.bufq_depth; bufq++)
        {
            for(ch = 0; ch < sensorObj->num_cameras_enabled; ch++)
            {
                vx_reference ref = vxGetObjectArrayItem(obj->output2.arr[bufq], ch);
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
                        /* Currently the vx_image buffers are allocated in one shot for multiple planes.
                            So if we are freeing this buffer then we need to get only the first plane
                            pointer address but add up the all the sizes to free the entire buffer */
                        vx_uint32 freeSize = 0;
                        for(ctr = 0; ctr < numEntries; ctr++)
                        {
                            freeSize += size[ctr];
                        }

                        if(virtAddr[0] != NULL)
                        {
                            TIOVX_MODULE_PRINTF("[VISS-MODULE] Freeing output2, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
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

    /* Free output3 handles */
    if(obj->output_select[3] == TIOVX_VISS_MODULE_OUTPUT_EN)
    {
        for(bufq = 0; bufq < obj->output3.bufq_depth; bufq++)
        {
            for(ch = 0; ch < sensorObj->num_cameras_enabled; ch++)
            {
                vx_reference ref = vxGetObjectArrayItem(obj->output3.arr[bufq], ch);
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
                        /* Currently the vx_image buffers are allocated in one shot for multiple planes.
                            So if we are freeing this buffer then we need to get only the first plane
                            pointer address but add up the all the sizes to free the entire buffer */
                        vx_uint32 freeSize = 0;
                        for(ctr = 0; ctr < numEntries; ctr++)
                        {
                            freeSize += size[ctr];
                        }

                        if(virtAddr[0] != NULL)
                        {
                            TIOVX_MODULE_PRINTF("[VISS-MODULE] Freeing output3, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
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

    /* Free output4 handles */
    if(obj->output_select[4] == TIOVX_VISS_MODULE_OUTPUT_EN)
    {
        for(bufq = 0; bufq < obj->output4.bufq_depth; bufq++)
        {
            for(ch = 0; ch < sensorObj->num_cameras_enabled; ch++)
            {
                vx_reference ref = vxGetObjectArrayItem(obj->output4.arr[bufq], ch);
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
                        /* Currently the vx_image buffers are allocated in one shot for multiple planes.
                            So if we are freeing this buffer then we need to get only the first plane
                            pointer address but add up the all the sizes to free the entire buffer */
                        vx_uint32 freeSize = 0;
                        for(ctr = 0; ctr < numEntries; ctr++)
                        {
                            freeSize += size[ctr];
                        }

                        if(virtAddr[0] != NULL)
                        {
                            TIOVX_MODULE_PRINTF("[VISS-MODULE] Freeing output4, bufq=%d, ch=%d, addr = 0x%016lX, size = %d \n", bufq, ch, (vx_uint64)virtAddr[0], freeSize);
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
        VX_PRINT(VX_ZONE_ERROR, "tivxReferenceExportHandle() failed.\n");
    }

    return status;
}

vx_status tiovx_viss_module_add_write_output_node(vx_graph graph, TIOVXVISSModuleObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_image output_img = (vx_image)vxGetObjectArrayItem(obj->output2.arr[0], 0);
    obj->img_write_node = tivxWriteImageNode(graph, output_img, obj->file_path, obj->img_file_prefix);
    vxReleaseImage(&output_img);

    status = vxGetStatus((vx_reference)obj->img_write_node);
    if((vx_status)VX_SUCCESS == status)
    {
        vxSetNodeTarget(obj->img_write_node, VX_TARGET_STRING, TIVX_TARGET_A72_0);

        vx_bool replicate[] = { vx_true_e, vx_false_e, vx_false_e};
        vxReplicateNode(graph, obj->img_write_node, replicate, 3);
    }
    else
    {
        TIOVX_MODULE_ERROR("[VISS-MODULE] Unable to create node to write VISS output! \n");
    }

    if((vx_status)VX_SUCCESS == status)
    {
        vx_user_data_object output_h3a = (vx_user_data_object)vxGetObjectArrayItem(obj->h3a_stats_arr[0], 0);

        obj->h3a_write_node = tivxWriteUserDataObjectNode(graph, output_h3a, obj->file_path, obj->h3a_file_prefix);
        vxReleaseUserDataObject(&output_h3a);

        status = vxGetStatus((vx_reference)obj->h3a_write_node);
        if((vx_status)VX_SUCCESS == status)
        {
            vxSetNodeTarget(obj->h3a_write_node, VX_TARGET_STRING, TIVX_TARGET_A72_0);

            vx_bool replicate[] = { vx_true_e, vx_false_e, vx_false_e};
            vxReplicateNode(graph, obj->h3a_write_node, replicate, 3);
        }
        else
        {
            printf("[VISS-MODULE] Unable to create node to write H3A stats! \n");
        }
    }

    return (status);
}

vx_status tiovx_viss_module_send_write_output_cmd(TIOVXVISSModuleObj *obj, vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip)
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

        status = tivxNodeSendCommand(obj->img_write_node, TIVX_CONTROL_CMD_SEND_TO_ALL_REPLICATED_NODES,
                                 TIVX_FILEIO_CMD_SET_FILE_WRITE,
                                 refs, 1u);

        if(VX_SUCCESS != status)
        {
            TIOVX_MODULE_ERROR("[VISS-MODULE] Img Write Node send command failed!\n");
        }

        TIOVX_MODULE_PRINTF("[VISS-MODULE] Img Write node send command success!\n");
    }
    if((vx_status)VX_SUCCESS == status)
    {
        vx_reference refs[2];

        refs[0] = (vx_reference)obj->write_cmd;

        status = tivxNodeSendCommand(obj->h3a_write_node, TIVX_CONTROL_CMD_SEND_TO_ALL_REPLICATED_NODES,
                                 TIVX_FILEIO_CMD_SET_FILE_WRITE,
                                 refs, 1u);

        if(VX_SUCCESS != status)
        {
            TIOVX_MODULE_ERROR("[VISS-MODULE] H3A Write Node send command failed!\n");
        }

        TIOVX_MODULE_PRINTF("[VISS-MODULE] H3A Write node send command success!\n");
    }
    return (status);
}
