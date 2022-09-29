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
#include "tiovx_sensor_module.h"
#include "tiovx_ldc_module.h"

#define APP_BUFQ_DEPTH   (1)

#define INPUT_WIDTH  (1936)
#define INPUT_HEIGHT (1096)

#define OUTPUT_WIDTH  (1920)
#define OUTPUT_HEIGHT (1080)

#define LDC_TABLE_WIDTH     (1920)
#define LDC_TABLE_HEIGHT    (1080)
#define LDC_DS_FACTOR       (2)

typedef struct {

    /* OpenVX references */
    vx_context context;

    vx_graph   graph;

    SensorObj  sensorObj;

    TIOVXLDCModuleObj  ldcObj;

} AppObj;

static AppObj gAppObj;

static vx_status app_init(AppObj *obj);
static void app_deinit(AppObj *obj);
static vx_status app_create_graph(AppObj *obj);
static vx_status app_verify_graph(AppObj *obj);
static vx_status app_run_graph(AppObj *obj);
static void app_delete_graph(AppObj *obj);

vx_status app_modules_ldc_test(vx_int32 argc, vx_char* argv[])
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
        TIOVXLDCModuleObj *ldcObj = &obj->ldcObj;

        SensorObj *sensorObj = &obj->sensorObj;
        tiovx_querry_sensor(sensorObj);
        tiovx_init_sensor(sensorObj,"SENSOR_SONY_IMX390_UB953_D3");

        snprintf(ldcObj->dcc_config_file_path, TIVX_FILEIO_FILE_PATH_LENGTH, "%s", "/opt/imaging/imx390/dcc_ldc_wdr.bin");
        snprintf(ldcObj->lut_file_path, TIVX_FILEIO_FILE_PATH_LENGTH, "%s", "/opt/edgeai-tiovx-modules/data/input/imx390_ldc_lut_1920x1080.bin");

        ldcObj->ldc_mode = TIOVX_MODULE_LDC_OP_MODE_DCC_DATA; //TIOVX_MODULE_LDC_OP_MODE_MESH_IMAGE
        ldcObj->en_out_image_write = 0;
        ldcObj->en_output1 = 0;

        ldcObj->input.bufq_depth = APP_BUFQ_DEPTH;
        ldcObj->input.color_format = VX_DF_IMAGE_NV12;
        ldcObj->input.width = INPUT_WIDTH;
        ldcObj->input.height = INPUT_HEIGHT;

        ldcObj->output0.bufq_depth = APP_BUFQ_DEPTH;
        ldcObj->output0.color_format = VX_DF_IMAGE_NV12;
        ldcObj->output0.width = OUTPUT_WIDTH;
        ldcObj->output0.height = OUTPUT_HEIGHT;

        ldcObj->table_width  = LDC_TABLE_WIDTH;
        ldcObj->table_height = LDC_TABLE_HEIGHT;
        ldcObj->ds_factor    = LDC_DS_FACTOR;

        /* Initialize modules */
        status = tiovx_ldc_module_init(obj->context, ldcObj, sensorObj);
        APP_PRINTF("LDC Init Done! \n");
    }

    return status;
}

static void app_deinit(AppObj *obj)
{
    tiovx_deinit_sensor(&obj->sensorObj);

    tiovx_ldc_module_deinit(&obj->ldcObj);

    tivxHwaUnLoadKernels(obj->context);

    vxReleaseContext(&obj->context);
}

static void app_delete_graph(AppObj *obj)
{
    tiovx_ldc_module_delete(&obj->ldcObj);

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
        status = tiovx_ldc_module_create(obj->graph, &obj->ldcObj, NULL, TIVX_TARGET_VPAC_LDC1);
    }

    graph_parameter_index = 0;
    if((vx_status)VX_SUCCESS == status)
    {

        status = add_graph_parameter_by_node_index(obj->graph, obj->ldcObj.node, 6);

        obj->ldcObj.input.graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->ldcObj.input.image_handle[0];
        graph_parameter_index++;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = add_graph_parameter_by_node_index(obj->graph, obj->ldcObj.node, 7);

        obj->ldcObj.output0.graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->ldcObj.output0.image_handle[0];
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
        status = tiovx_ldc_module_release_buffers(&obj->ldcObj);
    }

    return status;
}

static vx_status app_run_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    char * input_filename = "/opt/edgeai-tiovx-modules/data/input/imx390_fisheye_1936x1096_nv12.yuv";
    char * output_filename = "/opt/edgeai-tiovx-modules/data/output/imx390_rectified_1920x1080_nv12.yuv";

    vx_image input_o, output_o;

    TIOVXLDCModuleObj *ldcObj = &obj->ldcObj;
    vx_int32 bufq;
    uint32_t num_refs;

    void *inAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    void *outAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};

    vx_uint32 inSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32 outSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];

    allocate_image_buffers(&ldcObj->input, inAddr, inSizes);
    allocate_image_buffers(&ldcObj->output0, outAddr, outSizes);

    bufq = 0;

    assign_image_buffers(&ldcObj->input, inAddr[bufq], inSizes[bufq], bufq);
    assign_image_buffers(&ldcObj->output0, outAddr[bufq], outSizes[bufq], bufq);

    readImage(input_filename, ldcObj->input.image_handle[0]);

    APP_PRINTF("Enqueueing input buffers!\n");
    vxGraphParameterEnqueueReadyRef(obj->graph, 0, (vx_reference*)&ldcObj->input.image_handle[0], 1);
    APP_PRINTF("Enqueueing output buffers!\n");
    vxGraphParameterEnqueueReadyRef(obj->graph, 1, (vx_reference*)&ldcObj->output0.image_handle[0], 1);

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

    writeImage(output_filename, ldcObj->output0.image_handle[0]);

    release_image_buffers(&ldcObj->input, inAddr[bufq], inSizes[bufq], bufq);
    release_image_buffers(&ldcObj->output0, outAddr[bufq], outSizes[bufq], bufq);

    delete_image_buffers(&ldcObj->input, inAddr, inSizes);
    delete_image_buffers(&ldcObj->output0, outAddr, outSizes);

    return status;
}

