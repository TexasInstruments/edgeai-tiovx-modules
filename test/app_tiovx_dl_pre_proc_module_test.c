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

#include "app_common.h"
#include "tiovx_dl_pre_proc_module.h"

#define APP_BUFQ_DEPTH   (1)
#define APP_NUM_CH       (1)

#define IMAGE_WIDTH      (640)
#define IMAGE_HEIGHT     (480)

typedef struct {

    /* OpenVX references */
    vx_context context;

    vx_graph   graph;

    TIOVXDLPreProcModuleObj  dlPreProcObj;

} AppObj;

AppObj gAppObj;

static vx_status app_init(AppObj *obj);
static void app_deinit(AppObj *obj);
static vx_status app_create_graph(AppObj *obj);
static vx_status app_verify_graph(AppObj *obj);
static vx_status app_run_graph(AppObj *obj);
static void app_delete_graph(AppObj *obj);

vx_status app_modules_dl_pre_proc_test(vx_int32 argc, vx_char* argv[])
{
    AppObj *obj = &gAppObj;
    vx_status status = VX_SUCCESS;

    status = app_init(obj);
    APP_PRINTF("App Init Done! \n");

    if(status == VX_SUCCESS)
    {
        status = app_create_graph(obj);
        APP_PRINTF("App Create Graph Done! \n");
    }
    if(status == VX_SUCCESS)
    {
        status = app_verify_graph(obj);
        APP_PRINTF("App Verify Graph Done! \n");
    }
    if (status == VX_SUCCESS)
    {
        status = app_run_graph(obj);
        APP_PRINTF("App Run Graph Done! \n");
    }

    app_delete_graph(obj);
    APP_PRINTF("App Delete Graph Done! \n");

    app_deinit(obj);
    APP_PRINTF("App De-init Done! \n");

    return status;
}

static vx_status app_init(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    /* Create OpenVx Context */
    obj->context = vxCreateContext();
    status = vxGetStatus((vx_reference) obj->context);
    if(status == VX_SUCCESS)
    {
        tivxHwaLoadKernels(obj->context);
        tivxImgProcLoadKernels(obj->context);
    }

    if(status == VX_SUCCESS)
    {
        TIOVXDLPreProcModuleObj *dlPreProcObj = &obj->dlPreProcObj;

        dlPreProcObj->params.channel_order = TIVX_DL_PRE_PROC_CHANNEL_ORDER_NHWC;
        dlPreProcObj->params.tensor_format = TIVX_DL_PRE_PROC_TENSOR_FORMAT_RGB;

        dlPreProcObj->params.scale[0] = 1.0; //For R or Y plane
        dlPreProcObj->params.scale[1] = 1.0; //For G or U plane
        dlPreProcObj->params.scale[2] = 1.0; //For B or V plane

        dlPreProcObj->params.mean[0] = 0.0; //For R or Y plane
        dlPreProcObj->params.mean[1] = 0.0; //For G or U plane
        dlPreProcObj->params.mean[2] = 0.0; //For B or V plane

        /* Crop is not supported in 1.0 release */
        dlPreProcObj->params.crop[0] = 0; //Top
        dlPreProcObj->params.crop[1] = 0; //Bottom
        dlPreProcObj->params.crop[2] = 0; //Left
        dlPreProcObj->params.crop[3] = 0; //Right

        dlPreProcObj->num_channels = APP_NUM_CH;
        dlPreProcObj->input.bufq_depth = APP_BUFQ_DEPTH;
        dlPreProcObj->input.color_format = VX_DF_IMAGE_NV12;
        dlPreProcObj->input.width = IMAGE_WIDTH;
        dlPreProcObj->input.height = IMAGE_HEIGHT;

        dlPreProcObj->output.bufq_depth = APP_BUFQ_DEPTH;
        dlPreProcObj->output.datatype = VX_TYPE_UINT8;
        dlPreProcObj->output.num_dims = 3;
        /* This can be updated based on NCHW or NHWC */
        dlPreProcObj->output.dim_sizes[0] = IMAGE_WIDTH;
        dlPreProcObj->output.dim_sizes[1] = IMAGE_HEIGHT;
        dlPreProcObj->output.dim_sizes[2] = 3;

        dlPreProcObj->en_out_tensor_write = 0;

        /* Initialize modules */
        status = tiovx_dl_pre_proc_module_init(obj->context, dlPreProcObj);
        APP_PRINTF("DLPreProc Init Done! \n");
    }

    return status;
}

static void app_deinit(AppObj *obj)
{
    tiovx_dl_pre_proc_module_deinit(&obj->dlPreProcObj);

    tivxImgProcUnLoadKernels(obj->context);

    tivxHwaUnLoadKernels(obj->context);

    vxReleaseContext(&obj->context);
}

static void app_delete_graph(AppObj *obj)
{
    tiovx_dl_pre_proc_module_delete(&obj->dlPreProcObj);

    vxReleaseGraph(&obj->graph);
}

static vx_status app_create_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[8];
    vx_int32 graph_parameter_index;

    obj->graph = vxCreateGraph(obj->context);
    status = vxGetStatus((vx_reference)obj->graph);

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_dl_pre_proc_module_create(obj->graph, &obj->dlPreProcObj, NULL, TIVX_TARGET_DSP1);
    }

    graph_parameter_index = 0;
    if((vx_status)VX_SUCCESS == status)
    {
        status = add_graph_parameter_by_node_index(obj->graph, obj->dlPreProcObj.node, 1);
        obj->dlPreProcObj.input.graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->dlPreProcObj.input.image_handle[0];
        graph_parameter_index++;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = add_graph_parameter_by_node_index(obj->graph, obj->dlPreProcObj.node, 2);
        obj->dlPreProcObj.output.graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->dlPreProcObj.output.tensor_handle[0];
        graph_parameter_index++;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = vxSetGraphScheduleConfig(obj->graph,
                    VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL,
                    graph_parameter_index,
                    graph_parameters_queue_params_list);
    }

    return status;
}

static vx_status app_verify_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    status = vxVerifyGraph(obj->graph);

    APP_PRINTF("App Verify Graph Done!\n");

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_dl_pre_proc_module_release_buffers(&obj->dlPreProcObj);
    }

    return status;
}

static vx_status app_run_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    char * input_filename = "/opt/edgeai-tiovx-modules/data/output/baboon.yuv";
    char * output_filename = "/opt/edgeai-tiovx-modules/data/output/dl-pre-proc-output";

    vx_image input_o, output_o;

    TIOVXDLPreProcModuleObj *dlPreProcObj = &obj->dlPreProcObj;
    vx_int32 bufq;
    uint32_t num_refs;

    void *inAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    void *outAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};

    vx_uint32 inSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32 outSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];

    /* These can be moved to app_init() */
    allocate_image_buffers(&dlPreProcObj->input, inAddr, inSizes);
    allocate_tensor_buffers(&dlPreProcObj->output, outAddr, outSizes);

    bufq = 0;
    assign_image_buffers(&dlPreProcObj->input, inAddr[bufq], inSizes[bufq], bufq);
    assign_tensor_buffers(&dlPreProcObj->output, outAddr[bufq], outSizes[bufq], bufq);

    if(obj->dlPreProcObj.input.color_format == VX_DF_IMAGE_NV12)
    {
        readImage(input_filename, dlPreProcObj->input.image_handle[0]);
    }
    else if (obj->dlPreProcObj.input.color_format == VX_DF_IMAGE_RGB)
    {
        tivx_utils_load_vximage_from_bmpfile (dlPreProcObj->input.image_handle[0], input_filename, vx_false_e);
    }

    APP_PRINTF("Enqueueing input buffers!\n");
    vxGraphParameterEnqueueReadyRef(obj->graph, 0, (vx_reference*)&dlPreProcObj->input.image_handle[0], 1);
    APP_PRINTF("Enqueueing output buffers!\n");
    vxGraphParameterEnqueueReadyRef(obj->graph, 1, (vx_reference*)&dlPreProcObj->output.tensor_handle[0], 1);

    APP_PRINTF("Processing!\n");
    status = vxScheduleGraph(obj->graph);
    if((vx_status)VX_SUCCESS != status) {
      APP_PRINTF("Schedule Graph failed: %d!\n", status);
    }
    status = vxWaitGraph(obj->graph);
    if((vx_status)VX_SUCCESS != status) {
      APP_PRINTF("Wait Graph failed: %d!\n", status);
    }

    vxGraphParameterDequeueDoneRef(obj->graph, 0, (vx_reference*)&input_o, 1, &num_refs);
    vxGraphParameterDequeueDoneRef(obj->graph, 1, (vx_reference*)&output_o, 1, &num_refs);

    writeTensor(output_filename, dlPreProcObj->output.tensor_handle[0]);

    release_image_buffers(&dlPreProcObj->input, inAddr[bufq], inSizes[bufq], bufq);
    release_tensor_buffers(&dlPreProcObj->output, outAddr[bufq], outSizes[bufq], bufq);

    /* These can move to deinit() */
    delete_image_buffers(&dlPreProcObj->input, inAddr, inSizes);
    delete_tensor_buffers(&dlPreProcObj->output, outAddr, outSizes);

    return status;
}

