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
#include "tiovx_sensor_module.h"
#include "tiovx_viss_module.h"

#define APP_BUFQ_DEPTH   (1)

#define INPUT_WIDTH  (1936)
#define INPUT_HEIGHT (1096)

#define OUTPUT_WIDTH  (INPUT_WIDTH)
#define OUTPUT_HEIGHT (INPUT_HEIGHT)

typedef struct {

    /* OpenVX references */
    vx_context context;

    vx_graph   graph;

    SensorObj  sensorObj;

    TIOVXVISSModuleObj  vissObj;

} AppObj;

AppObj gAppObj;

static vx_status app_init(AppObj *obj);
static void app_deinit(AppObj *obj);
static vx_status app_create_graph(AppObj *obj);
static vx_status app_verify_graph(AppObj *obj);
static vx_status app_run_graph(AppObj *obj);
static void app_delete_graph(AppObj *obj);

static vx_status update2Aresults(vx_user_data_object ae_awb_result);

vx_status app_modules_viss_test(vx_int32 argc, vx_char* argv[])
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
        TIOVXVISSModuleObj *vissObj = &obj->vissObj;

        SensorObj *sensorObj = &obj->sensorObj;
        tiovx_querry_sensor(sensorObj);
        tiovx_init_sensor(sensorObj,"SENSOR_SONY_IMX390_UB953_D3");

        snprintf(vissObj->dcc_config_file_path, TIVX_FILEIO_FILE_PATH_LENGTH, "%s", "/opt/imaging/imx390/dcc_viss_wdr.bin");

        vissObj->input.bufq_depth = APP_BUFQ_DEPTH;
        /* information here is hardcoded for IMX390 sensor */
        /* Typically these information should be obtained by querying the sensor */
        vissObj->input.params.width  = INPUT_WIDTH;
        vissObj->input.params.height = INPUT_HEIGHT;
        vissObj->input.params.num_exposures = 1;
        vissObj->input.params.line_interleaved = vx_false_e;
        vissObj->input.params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
        vissObj->input.params.format[0].msb = 11;
        vissObj->input.params.meta_height_before = 0;
        vissObj->input.params.meta_height_after = 0;

        vissObj->ae_awb_result_bufq_depth = APP_BUFQ_DEPTH;

        /* Enable NV12 output from VISS which can be tapped from output mux 2*/
        vissObj->output_select[0] = TIOVX_VISS_MODULE_OUTPUT_NA;
        vissObj->output_select[1] = TIOVX_VISS_MODULE_OUTPUT_NA;
        vissObj->output_select[2] = TIOVX_VISS_MODULE_OUTPUT_EN;
        vissObj->output_select[3] = TIOVX_VISS_MODULE_OUTPUT_NA;
        vissObj->output_select[4] = TIOVX_VISS_MODULE_OUTPUT_NA;

        /* As we are selecting output2, specify output2 image properties */
        vissObj->output2.bufq_depth   = APP_BUFQ_DEPTH;
        vissObj->output2.color_format = VX_DF_IMAGE_NV12;
        vissObj->output2.width        = OUTPUT_WIDTH;
        vissObj->output2.height       = OUTPUT_HEIGHT;

        vissObj->h3a_stats_bufq_depth = APP_BUFQ_DEPTH;

        /* Initialize modules */
        status = tiovx_viss_module_init(obj->context, vissObj, sensorObj);
        APP_PRINTF("VISS Init Done! \n");
    }

    return status;
}

static void app_deinit(AppObj *obj)
{
    tiovx_deinit_sensor(&obj->sensorObj);

    tiovx_viss_module_deinit(&obj->vissObj);

    tivxHwaUnLoadKernels(obj->context);

    vxReleaseContext(&obj->context);
}

static void app_delete_graph(AppObj *obj)
{
    tiovx_viss_module_delete(&obj->vissObj);

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
        status = tiovx_viss_module_create(obj->graph, &obj->vissObj, NULL, NULL, TIVX_TARGET_VPAC_VISS1);
    }

    graph_parameter_index = 0;

    if((vx_status)VX_SUCCESS == status)
    {
        status = add_graph_parameter_by_node_index(obj->graph, obj->vissObj.node, 1);

        obj->vissObj.ae_awb_result_graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->vissObj.ae_awb_result_handle[0];
        graph_parameter_index++;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = add_graph_parameter_by_node_index(obj->graph, obj->vissObj.node, 3);

        obj->vissObj.input.graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->vissObj.input.image_handle[0];
        graph_parameter_index++;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = add_graph_parameter_by_node_index(obj->graph, obj->vissObj.node, 6);

        obj->vissObj.output2.graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->vissObj.output2.image_handle[0];
        graph_parameter_index++;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = add_graph_parameter_by_node_index(obj->graph, obj->vissObj.node, 9);

        obj->vissObj.h3a_stats_graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->vissObj.h3a_stats_handle[0];
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
        status = tiovx_viss_module_release_buffers(&obj->vissObj);
    }

    return status;
}

static vx_status app_run_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    char * input_filename = "/opt/edgeai-tiovx-modules/data/input/imx390_raw_image_1936x1096_16bpp_exp0.raw";
    char * output_filename = "/opt/edgeai-tiovx-modules/data/output/imx390_fisheye_1936x1096_nv12.yuv";

    tivx_raw_image input_o;
    vx_image output_o;
    vx_user_data_object aewb_o;
    vx_user_data_object h3a_o;

    TIOVXVISSModuleObj *vissObj = &obj->vissObj;
    vx_int32 bufq;
    uint32_t num_refs;

    void *inAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    void *outAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    void *aewbAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    void *h3aAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};

    vx_uint32 inSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32 outSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32 aewbSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32 h3aSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];

    allocate_raw_image_buffers(&vissObj->input, inAddr, inSizes);
    allocate_image_buffers(&vissObj->output2, outAddr, outSizes);
    allocate_user_data_buffers(vissObj->ae_awb_result_arr, aewbAddr, aewbSizes, APP_BUFQ_DEPTH);
    allocate_user_data_buffers(vissObj->h3a_stats_arr, h3aAddr, h3aSizes, APP_BUFQ_DEPTH);

    bufq = 0;

    assign_raw_image_buffers(&vissObj->input, inAddr[bufq], inSizes[bufq], bufq);
    assign_image_buffers(&vissObj->output2, outAddr[bufq], outSizes[bufq], bufq);
    assign_user_data_buffers(vissObj->ae_awb_result_arr, aewbAddr[bufq], aewbSizes[bufq], bufq);
    assign_user_data_buffers(vissObj->h3a_stats_arr, h3aAddr[bufq], h3aSizes[bufq], bufq);

    readRawImage(input_filename, vissObj->input.image_handle[0]);
    update2Aresults(vissObj->ae_awb_result_handle[0]);

    APP_PRINTF("Enqueueing input raw buffers!\n");
    vxGraphParameterEnqueueReadyRef(obj->graph, vissObj->input.graph_parameter_index, (vx_reference*)&vissObj->input.image_handle[0], 1);

    APP_PRINTF("Enqueueing ae-awb buffers!\n");
    vxGraphParameterEnqueueReadyRef(obj->graph, vissObj->ae_awb_result_graph_parameter_index, (vx_reference*)&vissObj->ae_awb_result_handle[0], 1);

    APP_PRINTF("Enqueueing output image buffers!\n");
    vxGraphParameterEnqueueReadyRef(obj->graph, vissObj->output2.graph_parameter_index, (vx_reference*)&vissObj->output2.image_handle[0], 1);

    APP_PRINTF("Enqueueing h3a stats buffers!\n");
    vxGraphParameterEnqueueReadyRef(obj->graph, vissObj->h3a_stats_graph_parameter_index, (vx_reference*)&vissObj->h3a_stats_handle[0], 1);

    APP_PRINTF("Processing!\n");
    status = vxScheduleGraph(obj->graph);
    if((vx_status)VX_SUCCESS != status) {
      APP_PRINTF("Schedule Graph failed: %d!\n", status);
    }
    status = vxWaitGraph(obj->graph);
    if((vx_status)VX_SUCCESS != status) {
      APP_PRINTF("Wait Graph failed: %d!\n", status);
    }

    vxGraphParameterDequeueDoneRef(obj->graph, vissObj->input.graph_parameter_index, (vx_reference*)&input_o, 1, &num_refs);
    vxGraphParameterDequeueDoneRef(obj->graph, vissObj->output2.graph_parameter_index, (vx_reference*)&output_o, 1, &num_refs);
    vxGraphParameterDequeueDoneRef(obj->graph, vissObj->ae_awb_result_graph_parameter_index, (vx_reference*)&aewb_o, 1, &num_refs);
    vxGraphParameterDequeueDoneRef(obj->graph, vissObj->h3a_stats_graph_parameter_index, (vx_reference*)&h3a_o, 1, &num_refs);

    writeImage(output_filename, vissObj->output2.image_handle[0]);
    //writeUserData(output_filename, vissObj->h3a_stats_handle[0]);

    release_raw_image_buffers(&vissObj->input, inAddr[bufq], inSizes[bufq], bufq);
    release_image_buffers(&vissObj->output2, outAddr[bufq], outSizes[bufq], bufq);
    release_user_data_buffers(vissObj->ae_awb_result_arr, aewbAddr[bufq], aewbSizes[bufq], bufq);
    release_user_data_buffers(vissObj->h3a_stats_arr, h3aAddr[bufq], h3aSizes[bufq], bufq);

    delete_raw_image_buffers(&vissObj->input, inAddr, inSizes);
    delete_image_buffers(&vissObj->output2, outAddr, outSizes);
    delete_user_data_buffers(vissObj->ae_awb_result_arr, aewbAddr, aewbSizes, bufq);
    delete_user_data_buffers(vissObj->h3a_stats_arr, h3aAddr, h3aSizes, bufq);

    return status;
}

static vx_status update2Aresults(vx_user_data_object ae_awb_result)
{
    vx_status status = VX_SUCCESS;

    status = vxGetStatus((vx_reference)ae_awb_result);

    if((vx_status)VX_SUCCESS == status)
    {
        uint8_t * data_buf;
        vx_map_id ae_awb_result_map_id;

        vxMapUserDataObject(
            ae_awb_result,
            0,
            sizeof(tivx_ae_awb_params_t),
            &ae_awb_result_map_id,
            (void **)&data_buf,
            VX_WRITE_ONLY,
            VX_MEMORY_TYPE_HOST,
            0
        );
		/* Currently just setting it to zero, but will be computed based on previous VISS h3a stats and updated */
        memset(data_buf, 0, sizeof(tivx_ae_awb_params_t));

        vxUnmapUserDataObject(ae_awb_result, ae_awb_result_map_id);
    }

    return status;
}