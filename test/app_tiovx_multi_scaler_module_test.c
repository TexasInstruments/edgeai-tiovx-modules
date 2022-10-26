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

#include <tiovx_utils.h>
#include "tiovx_multi_scaler_module.h"

#define APP_BUFQ_DEPTH   (1)
#define APP_NUM_CH       (1)
#define APP_NUM_OUTPUTS  (2)

#define INPUT_WIDTH  (640)
#define INPUT_HEIGHT (480)

typedef struct {

    /* OpenVX references */
    vx_context context;

    vx_graph   graph;

    TIOVXMultiScalerModuleObj  scalerObj;

} AppObj;

static AppObj gAppObj;

static vx_status app_init(AppObj *obj);
static void app_deinit(AppObj *obj);
static vx_status app_create_graph(AppObj *obj);
static vx_status app_verify_graph(AppObj *obj);
static vx_status app_run_graph(AppObj *obj);
static void app_delete_graph(AppObj *obj);

vx_status app_modules_scaler_test(vx_int32 argc, vx_char* argv[])
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
    }

    if(status == VX_SUCCESS)
    {
        TIOVXMultiScalerModuleObj *scalerObj = &obj->scalerObj;
        vx_int32 out;

        scalerObj->num_channels = APP_NUM_CH;
        scalerObj->num_outputs = APP_NUM_OUTPUTS;
        scalerObj->input.bufq_depth = APP_BUFQ_DEPTH;
        for(out = 0; out < APP_NUM_OUTPUTS; out++)
        {
            scalerObj->output[out].bufq_depth = APP_BUFQ_DEPTH;
        }
        scalerObj->interpolation_method = VX_INTERPOLATION_BILINEAR;
        scalerObj->color_format = VX_DF_IMAGE_NV12;

        scalerObj->input.width = INPUT_WIDTH;
        scalerObj->input.height = INPUT_HEIGHT;

        for(out = 0; out < APP_NUM_OUTPUTS; out++)
        {
            scalerObj->output[out].width = INPUT_WIDTH >> (out + 1);
            scalerObj->output[out].height = INPUT_HEIGHT >> (out + 1);
        }

        /* Initialize modules */
        status = tiovx_multi_scaler_module_init(obj->context, scalerObj);
        APP_PRINTF("Scaler Init Done! \n");
    }

    return status;
}

static void app_deinit(AppObj *obj)
{
    tiovx_multi_scaler_module_deinit(&obj->scalerObj);

    tivxHwaUnLoadKernels(obj->context);

    vxReleaseContext(&obj->context);
}

static void app_delete_graph(AppObj *obj)
{
    tiovx_multi_scaler_module_delete(&obj->scalerObj);

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
        status = tiovx_multi_scaler_module_create(obj->graph, &obj->scalerObj, NULL, TIVX_TARGET_VPAC_MSC1);
    }

    graph_parameter_index = 0;
    if((vx_status)VX_SUCCESS == status)
    {
        status = add_graph_parameter_by_node_index(obj->graph, obj->scalerObj.node, 0);
        obj->scalerObj.input.graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->scalerObj.input.image_handle[0];
        graph_parameter_index++;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = add_graph_parameter_by_node_index(obj->graph, obj->scalerObj.node, 1);
        obj->scalerObj.output[0].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->scalerObj.output[0].image_handle[0];
        graph_parameter_index++;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = add_graph_parameter_by_node_index(obj->graph, obj->scalerObj.node, 2);
        obj->scalerObj.output[1].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->scalerObj.output[1].image_handle[0];
        graph_parameter_index++;
    }

    if(status == VX_SUCCESS)
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
        status = tiovx_multi_scaler_module_update_filter_coeffs(&obj->scalerObj);
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_multi_scaler_module_release_buffers(&obj->scalerObj);
    }

    return status;
}

static vx_status app_run_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    char * input_filename = "/opt/edgeai-tiovx-modules/data/input/baboon_640x480_nv12.yuv";
    char * output0_filename = "/opt/edgeai-tiovx-modules/data/output/baboon_320x240_nv12_msc_out0.yuv";
    char * output1_filename = "/opt/edgeai-tiovx-modules/data/output/baboon_160x120_nv12_msc_out1.yuv";

    vx_image input_o, output0_o, output1_o;

    TIOVXMultiScalerModuleObj *scalerObj = &obj->scalerObj;
    vx_int32 bufq;
    uint32_t num_refs;

    void *inAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    void *out1Addr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    void *out2Addr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};

    vx_uint32 inSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32 out1Sizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32 out2Sizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];

    /* These can be moved to app_init() */
    allocate_image_buffers(&scalerObj->input, inAddr, inSizes);
    allocate_image_buffers(&scalerObj->output[0], out1Addr, out1Sizes);
    allocate_image_buffers(&scalerObj->output[1], out2Addr, out2Sizes);

    bufq = 0;
    assign_image_buffers(&scalerObj->input, inAddr[bufq], inSizes[bufq], bufq);
    assign_image_buffers(&scalerObj->output[0], out1Addr[bufq], out1Sizes[bufq], bufq);
    assign_image_buffers(&scalerObj->output[1], out2Addr[bufq], out2Sizes[bufq], bufq);

    readImage(input_filename, scalerObj->input.image_handle[0]);

    APP_PRINTF("Enqueueing input buffers!\n");
    vxGraphParameterEnqueueReadyRef(obj->graph, 0, (vx_reference*)&scalerObj->input.image_handle[0], 1);
    APP_PRINTF("Enqueueing output buffers!\n");
    vxGraphParameterEnqueueReadyRef(obj->graph, 1, (vx_reference*)&scalerObj->output[0].image_handle[0], 1);
    vxGraphParameterEnqueueReadyRef(obj->graph, 2, (vx_reference*)&scalerObj->output[1].image_handle[0], 1);

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
    vxGraphParameterDequeueDoneRef(obj->graph, 1, (vx_reference*)&output0_o, 1, &num_refs);
    vxGraphParameterDequeueDoneRef(obj->graph, 2, (vx_reference*)&output1_o, 1, &num_refs);

    writeImage(output0_filename, scalerObj->output[0].image_handle[0]);
    writeImage(output1_filename, scalerObj->output[1].image_handle[0]);

    release_image_buffers(&scalerObj->input, inAddr[bufq], inSizes[bufq], bufq);
    release_image_buffers(&scalerObj->output[0], out1Addr[bufq], out1Sizes[bufq], bufq);
    release_image_buffers(&scalerObj->output[1], out2Addr[bufq], out2Sizes[bufq], bufq);

    /* These can move to deinit() */
    delete_image_buffers(&scalerObj->input, inAddr, inSizes);
    delete_image_buffers(&scalerObj->output[0], out1Addr, out1Sizes);
    delete_image_buffers(&scalerObj->output[1], out2Addr, out2Sizes);

    return status;
}
