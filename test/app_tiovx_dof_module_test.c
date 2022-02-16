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
#include "tiovx_dof_module.h"

#define APP_BUFQ_DEPTH   (1)
#define APP_NUM_CH       (1)

#define IMAGE_WIDTH  (1280)
#define IMAGE_HEIGHT (720)

#define PYRAMID_LEVELS (3)

typedef struct {

    /* OpenVX references */
    vx_context context;

    vx_graph   graph;

    TIOVXDofModuleObj  dofObj;

} AppObj;

AppObj gAppObj;

static vx_status app_init(AppObj *obj);
static void app_deinit(AppObj *obj);
static vx_status app_create_graph(AppObj *obj);
static vx_status app_verify_graph(AppObj *obj);
static vx_status app_run_graph(AppObj *obj);
static void app_delete_graph(AppObj *obj);

vx_status app_modules_dof_test(vx_int32 argc, vx_char* argv[])
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
        TIOVXDofModuleObj *dofObj = &obj->dofObj;

        dofObj->input.bufq_depth = APP_BUFQ_DEPTH;
        dofObj->input.color_format = VX_DF_IMAGE_U8;
        dofObj->input.levels = PYRAMID_LEVELS;
        dofObj->input.scale = VX_SCALE_PYRAMID_HALF;

        dofObj->input_ref.bufq_depth = APP_BUFQ_DEPTH;
        dofObj->input_ref.color_format = VX_DF_IMAGE_U8;
        dofObj->input_ref.levels = PYRAMID_LEVELS;
        dofObj->input_ref.scale = VX_SCALE_PYRAMID_HALF;

        dofObj->input_flow_vector.bufq_depth = APP_BUFQ_DEPTH;
        dofObj->input_flow_vector.color_format = VX_DF_IMAGE_U32;

        dofObj->output_flow_vector.bufq_depth = APP_BUFQ_DEPTH;
        dofObj->output_flow_vector.color_format = VX_DF_IMAGE_U32;

        dofObj->output_distribution.bufq_depth = APP_BUFQ_DEPTH;
        dofObj->output_distribution.num_bins = 16;
        dofObj->output_distribution.offset = 0;
        dofObj->output_distribution.range = 16;

        dofObj->num_channels = APP_NUM_CH;
        dofObj->width = IMAGE_WIDTH;
        dofObj->height = IMAGE_HEIGHT;
        dofObj->enable_temporal_predicton_flow_vector = 0;
        dofObj->enable_output_distribution = 0;

        //Configure dof params
        tivx_dmpac_dof_params_init(&dofObj->params);

        if(dofObj->enable_temporal_predicton_flow_vector == 0)
        {
          dofObj->params.base_predictor[0] = TIVX_DMPAC_DOF_PREDICTOR_DELAY_LEFT;
          dofObj->params.base_predictor[1] = TIVX_DMPAC_DOF_PREDICTOR_PYR_COLOCATED;

          dofObj->params.inter_predictor[0] = TIVX_DMPAC_DOF_PREDICTOR_DELAY_LEFT;
          dofObj->params.inter_predictor[1] = TIVX_DMPAC_DOF_PREDICTOR_PYR_COLOCATED;
        }

        dofObj->params.vertical_search_range[0] = 48;
        dofObj->params.vertical_search_range[1] = 48;
        dofObj->params.horizontal_search_range = 191;
        dofObj->params.median_filter_enable = 1;
        dofObj->params.motion_smoothness_factor = 12;
        dofObj->params.motion_direction = 1; /* 0: for side camera */

        /* Initialize modules */
        status = tiovx_dof_module_init(obj->context, dofObj);
        APP_PRINTF("Dof Init Done! \n");
    }

    return status;
}

static void app_deinit(AppObj *obj)
{
    tiovx_dof_module_deinit(&obj->dofObj);

    tivxHwaUnLoadKernels(obj->context);

    vxReleaseContext(&obj->context);
}

static void app_delete_graph(AppObj *obj)
{
    tiovx_dof_module_delete(&obj->dofObj);

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
        status = tiovx_dof_module_create(obj->graph, &obj->dofObj, NULL, NULL, NULL, TIVX_TARGET_DMPAC_DOF);
    }

    graph_parameter_index = 0;
    if((vx_status)VX_SUCCESS == status)
    {
        status = add_graph_parameter_by_node_index(obj->graph, obj->dofObj.node, 3);
        obj->dofObj.input.graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->dofObj.input.pyramid_handle[0];
        graph_parameter_index++;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = add_graph_parameter_by_node_index(obj->graph, obj->dofObj.node, 4);
        obj->dofObj.input_ref.graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->dofObj.input_ref.pyramid_handle[0];
        graph_parameter_index++;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = add_graph_parameter_by_node_index(obj->graph, obj->dofObj.node, 8);
        obj->dofObj.output_flow_vector.graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->dofObj.output_flow_vector.image_handle[0];
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
        status = tiovx_dof_module_release_buffers(&obj->dofObj);
    }

    return status;
}

static vx_status app_run_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    char * input_filename_template = "/opt/edgeai-tiovx-modules/data/input/avp3_1280x720_0_pyramid_level_%d_u8.gray";
    char * input_ref_filename_template = "/opt/edgeai-tiovx-modules/data/input/avp3_1280x720_1_pyramid_level_%d_u8.gray";
    char * output_filename = "/opt/edgeai-tiovx-modules/data/output/output_dof_u32.raw";

    vx_pyramid  input_o, input_ref_o;
    vx_image output_flow_vector_o;

    TIOVXDofModuleObj *dofObj = &obj->dofObj;
    vx_int32 bufq;
    uint32_t num_refs;

    void *inAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    void *inRefAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    void *outAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};

    vx_uint32 inSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32 inRefSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32 outSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];

    /* These can be moved to app_init() */
    allocate_pyramid_buffers(&dofObj->input, inAddr, inSizes);
    allocate_pyramid_buffers(&dofObj->input_ref, inRefAddr, inRefSizes);
    allocate_image_buffers(&dofObj->output_flow_vector, outAddr, outSizes);

    bufq = 0;
    assign_pyramid_buffers(&dofObj->input, inAddr[bufq], inSizes[bufq], bufq);
    assign_pyramid_buffers(&dofObj->input_ref, inRefAddr[bufq], inRefSizes[bufq], bufq);
    assign_image_buffers(&dofObj->output_flow_vector, outAddr[bufq], outSizes[bufq], bufq);

    vx_image in_image;
    uint32_t i;
    char input_filename[100];
    for (i = 0; i < PYRAMID_LEVELS; i++ )
    {
        sprintf(input_filename, input_filename_template, i);

        in_image = vxGetPyramidLevel(dofObj->input.pyramid_handle[0], i);
        readImage(input_filename, in_image);
        vxReleaseImage(&in_image);

        sprintf(input_filename, input_ref_filename_template, i);

        in_image = vxGetPyramidLevel(dofObj->input_ref.pyramid_handle[0], i);
        readImage(input_filename, in_image);
        vxReleaseImage(&in_image);
    }

    APP_PRINTF("Enqueueing input buffers!\n");
    vxGraphParameterEnqueueReadyRef(obj->graph, 0, (vx_reference*)&dofObj->input.pyramid_handle[0], 1);
    vxGraphParameterEnqueueReadyRef(obj->graph, 1, (vx_reference*)&dofObj->input_ref.pyramid_handle[0], 1);
    APP_PRINTF("Enqueueing output buffers!\n");
    vxGraphParameterEnqueueReadyRef(obj->graph, 2, (vx_reference*)&dofObj->output_flow_vector.image_handle[0], 1);

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
    vxGraphParameterDequeueDoneRef(obj->graph, 1, (vx_reference*)&input_ref_o, 1, &num_refs);
    vxGraphParameterDequeueDoneRef(obj->graph, 2, (vx_reference*)&output_flow_vector_o, 1, &num_refs);

    writeImage(output_filename, dofObj->output_flow_vector.image_handle[0]);

    release_pyramid_buffers(&dofObj->input, inAddr[bufq], inSizes[bufq], bufq);
    release_pyramid_buffers(&dofObj->input_ref, inRefAddr[bufq], inRefSizes[bufq], bufq);
    release_image_buffers(&dofObj->output_flow_vector, outAddr[bufq], outSizes[bufq], bufq);

    /* These can move to deinit() */
    delete_pyramid_buffers(&dofObj->input, inAddr, inSizes);
    delete_pyramid_buffers(&dofObj->input_ref, inRefAddr, inRefSizes);
    delete_image_buffers(&dofObj->output_flow_vector, outAddr, outSizes);

    return status;
}

