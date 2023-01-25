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
#include "tiovx_dl_color_blend_module.h"

#define APP_BUFQ_DEPTH   (1)
#define APP_NUM_CH       (1)
#define APP_NUM_CLASSES  (8)

#define IMAGE_WIDTH  (640)
#define IMAGE_HEIGHT (480)

#define TENSOR_WIDTH  (IMAGE_WIDTH >> 1)
#define TENSOR_HEIGHT (IMAGE_HEIGHT >> 1)

typedef struct {

    /* OpenVX references */
    vx_context context;

    vx_graph   graph;

    TIOVXDLColorBlendModuleObj  dlColorBlendObj;

} AppObj;

static AppObj gAppObj;

static vx_status app_init(AppObj *obj);
static void app_deinit(AppObj *obj);
static vx_status app_create_graph(AppObj *obj);
static vx_status app_verify_graph(AppObj *obj);
static vx_status app_run_graph(AppObj *obj);
static void app_delete_graph(AppObj *obj);

vx_status app_modules_dl_color_blend_test(vx_int32 argc, vx_char* argv[])
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
        tivxEdgeaiImgProcLoadKernels(obj->context);
    }

    if(status == VX_SUCCESS)
    {
        TIOVXDLColorBlendModuleObj *dlColorBlendObj = &obj->dlColorBlendObj;

        dlColorBlendObj->num_channels = APP_NUM_CH;
        dlColorBlendObj->en_out_image_write = 0;

        dlColorBlendObj->params.use_color_map = 0;
        dlColorBlendObj->params.num_classes = APP_NUM_CLASSES;

        dlColorBlendObj->img_input.bufq_depth = APP_BUFQ_DEPTH;
        dlColorBlendObj->img_input.color_format = VX_DF_IMAGE_RGB;
        dlColorBlendObj->img_input.width = IMAGE_WIDTH;
        dlColorBlendObj->img_input.height = IMAGE_HEIGHT;

        dlColorBlendObj->tensor_input.bufq_depth = APP_BUFQ_DEPTH;
        dlColorBlendObj->tensor_input.datatype = VX_TYPE_UINT8;
        dlColorBlendObj->tensor_input.num_dims = 3;
        dlColorBlendObj->tensor_input.dim_sizes[0] = TENSOR_WIDTH;
        dlColorBlendObj->tensor_input.dim_sizes[1] = TENSOR_HEIGHT;
        dlColorBlendObj->tensor_input.dim_sizes[2] = 1;

        dlColorBlendObj->img_output.bufq_depth = APP_BUFQ_DEPTH;
        dlColorBlendObj->img_output.color_format = VX_DF_IMAGE_RGB;
        dlColorBlendObj->img_output.width = IMAGE_WIDTH;
        dlColorBlendObj->img_output.height = IMAGE_HEIGHT;

        /* Initialize modules */
        status = tiovx_dl_color_blend_module_init(obj->context, dlColorBlendObj);
        APP_PRINTF("DLColorBlend Init Done! \n");
    }

    return status;
}

static void app_deinit(AppObj *obj)
{
    tiovx_dl_color_blend_module_deinit(&obj->dlColorBlendObj);

    tivxEdgeaiImgProcUnLoadKernels(obj->context);

    tivxImgProcUnLoadKernels(obj->context);

    tivxHwaUnLoadKernels(obj->context);

    vxReleaseContext(&obj->context);
}

static void app_delete_graph(AppObj *obj)
{
    tiovx_dl_color_blend_module_delete(&obj->dlColorBlendObj);

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
        status = tiovx_dl_color_blend_module_create(obj->graph, &obj->dlColorBlendObj, NULL, NULL, TIVX_TARGET_A72_0);
    }

    graph_parameter_index = 0;
    if((vx_status)VX_SUCCESS == status)
    {
        status = add_graph_parameter_by_node_index(obj->graph, obj->dlColorBlendObj.node, 1);
        obj->dlColorBlendObj.img_input.graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->dlColorBlendObj.img_input.image_handle[0];
        graph_parameter_index++;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = add_graph_parameter_by_node_index(obj->graph, obj->dlColorBlendObj.node, 2);
        obj->dlColorBlendObj.tensor_input.graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->dlColorBlendObj.tensor_input.tensor_handle[0];
        graph_parameter_index++;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = add_graph_parameter_by_node_index(obj->graph, obj->dlColorBlendObj.node, 3);
        obj->dlColorBlendObj.img_output.graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->dlColorBlendObj.img_output.image_handle[0];
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
        status = tiovx_dl_color_blend_module_release_buffers(&obj->dlColorBlendObj);
    }

    return status;
}

static vx_status app_run_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    char * input_image_filename = "/opt/edgeai-tiovx-modules/data/input/baboon_640x480_rgb.bmp";
    char * input_tensor_filename = "/opt/edgeai-tiovx-modules/data/output/baboon_mask";
    char * output_image_filename = "/opt/edgeai-tiovx-modules/data/output/baboon_out_640x480_rgb.bmp";

    vx_image input_o, output_o;
    vx_tensor tensor_o;

    TIOVXDLColorBlendModuleObj *dlColorBlendObj = &obj->dlColorBlendObj;
    vx_int32 bufq;
    uint32_t num_refs;

    void *inImageAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    void *inTensorAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    void *outImageAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};

    vx_uint32 inImageSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32 inTensorSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32 outImageSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];

    /* These can be moved to app_init() */
    allocate_image_buffers(&dlColorBlendObj->img_input, inImageAddr, inImageSizes);
    allocate_tensor_buffers(&dlColorBlendObj->tensor_input, inTensorAddr, inTensorSizes);
    allocate_image_buffers(&dlColorBlendObj->img_output, outImageAddr, outImageSizes);

    bufq = 0;
    assign_image_buffers(&dlColorBlendObj->img_input, inImageAddr[bufq], inImageSizes[bufq], bufq);
    assign_tensor_buffers(&dlColorBlendObj->tensor_input, inTensorAddr[bufq], inTensorSizes[bufq], bufq);
    assign_image_buffers(&dlColorBlendObj->img_output, outImageAddr[bufq], outImageSizes[bufq], bufq);

    create_tensor_mask(dlColorBlendObj->tensor_input.tensor_handle[0], APP_NUM_CLASSES);
    writeTensor(input_tensor_filename, dlColorBlendObj->tensor_input.tensor_handle[0]);

    if(dlColorBlendObj->img_input.color_format == VX_DF_IMAGE_NV12)
    {
        readImage(input_image_filename, dlColorBlendObj->img_input.image_handle[0]);
    }
    else if (dlColorBlendObj->img_input.color_format == VX_DF_IMAGE_RGB)
    {
        tivx_utils_load_vximage_from_bmpfile (dlColorBlendObj->img_input.image_handle[0], input_image_filename, vx_false_e);
    }

    APP_PRINTF("Enqueueing input image buffers!\n");
    vxGraphParameterEnqueueReadyRef(obj->graph, obj->dlColorBlendObj.img_input.graph_parameter_index, (vx_reference*)&dlColorBlendObj->img_input.image_handle[0], 1);
    APP_PRINTF("Enqueueing input tensor buffers!\n");
    vxGraphParameterEnqueueReadyRef(obj->graph, obj->dlColorBlendObj.tensor_input.graph_parameter_index, (vx_reference*)&dlColorBlendObj->tensor_input.tensor_handle[0], 1);
    APP_PRINTF("Enqueueing output image buffers!\n");
    vxGraphParameterEnqueueReadyRef(obj->graph, obj->dlColorBlendObj.img_output.graph_parameter_index, (vx_reference*)&dlColorBlendObj->img_output.image_handle[0], 1);

    APP_PRINTF("Processing!\n");
    status = vxScheduleGraph(obj->graph);
    if((vx_status)VX_SUCCESS != status) {
      APP_PRINTF("Schedule Graph failed: %d!\n", status);
    }
    status = vxWaitGraph(obj->graph);
    if((vx_status)VX_SUCCESS != status) {
      APP_PRINTF("Wait Graph failed: %d!\n", status);
    }

    vxGraphParameterDequeueDoneRef(obj->graph, obj->dlColorBlendObj.img_input.graph_parameter_index, (vx_reference*)&input_o, 1, &num_refs);
    vxGraphParameterDequeueDoneRef(obj->graph, obj->dlColorBlendObj.tensor_input.graph_parameter_index, (vx_reference*)&tensor_o, 1, &num_refs);
    vxGraphParameterDequeueDoneRef(obj->graph, obj->dlColorBlendObj.img_output.graph_parameter_index, (vx_reference*)&output_o, 1, &num_refs);

    if(dlColorBlendObj->img_output.color_format == VX_DF_IMAGE_NV12)
    {
        writeImage(output_image_filename, dlColorBlendObj->img_output.image_handle[0]);
    }
    else if (dlColorBlendObj->img_input.color_format == VX_DF_IMAGE_RGB)
    {
        tivx_utils_save_vximage_to_bmpfile(output_image_filename, dlColorBlendObj->img_output.image_handle[0]);
    }

    release_image_buffers(&dlColorBlendObj->img_input, inImageAddr[bufq], inImageSizes[bufq], bufq);
    release_tensor_buffers(&dlColorBlendObj->tensor_input, inTensorAddr[bufq], inTensorSizes[bufq], bufq);
    release_image_buffers(&dlColorBlendObj->img_output, outImageAddr[bufq], outImageSizes[bufq], bufq);

    /* These can move to deinit() */
    delete_image_buffers(&dlColorBlendObj->img_input, inImageAddr, inImageSizes);
    delete_tensor_buffers(&dlColorBlendObj->tensor_input, inTensorAddr, inTensorSizes);
    delete_image_buffers(&dlColorBlendObj->img_output, outImageAddr, outImageSizes);

    return status;
}

