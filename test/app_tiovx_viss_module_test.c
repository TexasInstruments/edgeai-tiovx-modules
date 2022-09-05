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
#include "ti_2a_wrapper.h"

#define APP_BUFQ_DEPTH   (1)
#define NUM_ITERATIONS   (1)
#define INPUT_WIDTH_OV2312  (1600)
#define INPUT_HEIGHT_OV2312 (1300)

#if defined(SOC_AM62A)
    #define INPUT_WIDTH  INPUT_WIDTH_OV2312
    #define INPUT_HEIGHT INPUT_HEIGHT_OV2312
#else
    #define INPUT_WIDTH  (1920)
    #define INPUT_HEIGHT (1080)
#endif

#define OUTPUT_WIDTH  (INPUT_WIDTH)
#define OUTPUT_HEIGHT (INPUT_HEIGHT)

typedef struct {

    /* OpenVX references */
    vx_context context;

    vx_graph   graph;

    SensorObj  sensorObj;

    TIOVXVISSModuleObj  vissObj;

    tivx_aewb_config_t aewbConfig;

    TI_2A_wrapper aewbObj;

    sensor_config_get sensor_in_data;

    sensor_config_set sensor_out_data;

} AppObj;

static AppObj gAppObj;

static vx_status app_init(AppObj *obj);
static void app_deinit(AppObj *obj);
static vx_status app_create_graph(AppObj *obj);
static vx_status app_verify_graph(AppObj *obj);
static vx_status app_run_graph(AppObj *obj);
static void app_delete_graph(AppObj *obj);

#if defined(SOC_AM62A)
static int32_t OV2312_GetExpPrgFxn(IssAeDynamicParams *p_ae_dynPrms);
#else
static int32_t IMX219_GetExpPrgFxn(IssAeDynamicParams *p_ae_dynPrms);
#endif

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
#if defined(SOC_AM62A)
        tiovx_init_sensor(sensorObj,"SENSOR_OV2312_UB953_LI");
#else
        tiovx_init_sensor(sensorObj,"SENSOR_SONY_IMX219_RPI");
#endif    

        tivx_vpac_viss_params_init(&vissObj->params);

#if defined(SOC_AM62A)        
        snprintf(vissObj->dcc_config_file_path, TIVX_FILEIO_FILE_PATH_LENGTH, "%s", "/opt/imaging/ov2312/dcc_viss.bin");
#else
        snprintf(vissObj->dcc_config_file_path, TIVX_FILEIO_FILE_PATH_LENGTH, "%s", "/opt/imaging/imx219/dcc_viss.bin");
#endif

        vissObj->input.bufq_depth = APP_BUFQ_DEPTH;

        vissObj->input.params.width  = INPUT_WIDTH;
        vissObj->input.params.height = INPUT_HEIGHT;
        vissObj->input.params.num_exposures = 1;
        vissObj->input.params.line_interleaved = vx_false_e;
        vissObj->input.params.meta_height_before = 0;
        vissObj->input.params.meta_height_after = 0;
#if defined(SOC_AM62A)
        /* information here is hardcoded for OV2312 sensor */
        /* Typically this information should be obtained by querying the sensor */
        vissObj->input.params.format[0].pixel_container = TIVX_RAW_IMAGE_16_BIT;
        vissObj->input.params.format[0].msb = 9;
#else
        /* information here is hardcoded for IMX219 sensor */
        /* Typically this information should be obtained by querying the sensor */
        vissObj->input.params.format[0].pixel_container = TIVX_RAW_IMAGE_8_BIT;
        vissObj->input.params.format[0].msb = 7;
#endif

        vissObj->ae_awb_result_bufq_depth = APP_BUFQ_DEPTH;

#if defined(SOC_AM62A)

        vissObj->params.bypass_pcid = 0;

        vissObj->params.enable_ir_op = TIVX_VPAC_VISS_IR_DISABLE;
        vissObj->params.enable_bayer_op = TIVX_VPAC_VISS_BAYER_ENABLE;

        if(vissObj->params.enable_ir_op)
        {
            /* Enable RAW IR output from VISS which can be tapped from output mux 0 
            Only 8 bit IR and Packed 12 bit IR supported on Output 0
            For 12 bit output in 16 bit container use Output 2*/
            vissObj->output_select[0] = TIOVX_VISS_MODULE_OUTPUT_EN;
            vissObj->output_select[1] = TIOVX_VISS_MODULE_OUTPUT_NA;
            vissObj->output_select[2] = TIOVX_VISS_MODULE_OUTPUT_NA;
            vissObj->output_select[3] = TIOVX_VISS_MODULE_OUTPUT_NA;
            vissObj->output_select[4] = TIOVX_VISS_MODULE_OUTPUT_NA;

            /* As we are selecting output0, specify output0 image properties */
            vissObj->output0.bufq_depth   = APP_BUFQ_DEPTH;
            vissObj->output0.color_format = VX_DF_IMAGE_U8;
            /* For 8 bit IR output                      - VX_DF_IMAGE_U8
               For Packed 12 bit IR output              - TIVX_DF_IMAGE_P12 */
            vissObj->output0.width        = OUTPUT_WIDTH;
            vissObj->output0.height       = OUTPUT_HEIGHT;
        }
        if(vissObj->params.enable_bayer_op) 
        {
            /* Enable NV12 output from VISS which can be tapped from output mux 2*/
            vissObj->output_select[0] = TIOVX_VISS_MODULE_OUTPUT_NA;
            vissObj->output_select[1] = TIOVX_VISS_MODULE_OUTPUT_NA;
            vissObj->output_select[2] = TIOVX_VISS_MODULE_OUTPUT_EN;
            vissObj->output_select[3] = TIOVX_VISS_MODULE_OUTPUT_NA;
            vissObj->output_select[4] = TIOVX_VISS_MODULE_OUTPUT_NA;

            /* As we are selecting output2, specify output2 image properties */
            vissObj->output2.bufq_depth   = APP_BUFQ_DEPTH;
            vissObj->output2.color_format = VX_DF_IMAGE_NV12;
            /* For 12 IR bit output in 16 bit container - VX_DF_IMAGE_U16  */
            vissObj->output2.width        = OUTPUT_WIDTH;
            vissObj->output2.height       = OUTPUT_HEIGHT;
        }
        /* Even else would also work in our case, as in our use case we are just using
        either ir_enable or bayer_enable. But both flags i.e. ir_enable and bayer_enable 
        are required, as VPAC3L supports both IR and BAYER outputs simultaneously*/
#else
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
#endif

        vissObj->h3a_stats_bufq_depth = APP_BUFQ_DEPTH;

        /* Initialize modules */
        status = tiovx_viss_module_init(obj->context, vissObj, sensorObj);
        APP_PRINTF("VISS Init Done! \n");

#if defined(SOC_AM62A)
        char *aewb_dcc_file = "/opt/imaging/ov2312/dcc_2a.bin";
#else
        char *aewb_dcc_file = "/opt/imaging/imx219/dcc_2a.bin";
#endif
        FILE *aewb_fp = NULL;

        aewb_fp = fopen(aewb_dcc_file, "rb");
        if(aewb_fp == NULL)
        {
            APP_ERROR("Unable to open 2A DCC file path = %s \n", aewb_dcc_file);
            return VX_FAILURE;
        }

        fseek(aewb_fp, 0, SEEK_END);
        uint32_t aewb_dcc_file_size = ftell(aewb_fp);
        fseek(aewb_fp, 0, SEEK_SET);

        uint8_t *aewb_dcc_buf = (uint8_t *)malloc(aewb_dcc_file_size);
        if(aewb_dcc_buf == NULL)
        {
            APP_ERROR("Unable to allocate %d bytes for aewb_dcc_buf \n", aewb_dcc_file_size);
            return VX_FAILURE;
        }

        uint32_t read_size = fread(aewb_dcc_buf, sizeof(uint8_t), aewb_dcc_file_size, aewb_fp);

        if(read_size != aewb_dcc_file_size)
        {
            APP_ERROR("Bytes read %d bytes is not same as file size %d \n", read_size, aewb_dcc_file_size);
            return VX_FAILURE;
        }

        obj->aewbConfig.sensor_dcc_id       = sensorObj->sensorParams.dccId;
        obj->aewbConfig.sensor_img_format   = 0; /*!<Image Format : BAYER = 0x0, Rest unsupported */
        obj->aewbConfig.sensor_img_phase    = 3; /*!<Image Format : BGGR = 0, GBRG = 1, GRBG = 2, RGGB = 3 */

        obj->aewbConfig.ae_mode = ALGORITHMS_ISS_AE_AUTO; /*!<AWB Mode : 0 = Auto, 1 = Manual, 2 = Disabled */
        obj->aewbConfig.awb_mode = ALGORITHMS_ISS_AWB_AUTO; /*!<AE Mode : 0 = Auto, 1 = Manual, 2 = Disabled */

        obj->aewbConfig.awb_num_skip_frames = 0; /*!<0 = Process every frame */
        obj->aewbConfig.ae_num_skip_frames  = 0; /*!<0 = Process every frame */
        obj->aewbConfig.channel_id          = 0; /*!<channel ID */

        status = TI_2A_wrapper_create(&obj->aewbObj, &obj->aewbConfig, aewb_dcc_buf, aewb_dcc_file_size);
        if(status == VX_FAILURE)
        {
            APP_ERROR("Error during 2A create!\n");
        }
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

    TI_2A_wrapper_delete(&obj->aewbObj);

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
#if defined(SOC_AM62A)
        if(obj->vissObj.params.enable_ir_op)
        {
            status = add_graph_parameter_by_node_index(obj->graph, obj->vissObj.node, 4);
            obj->vissObj.output0.graph_parameter_index = graph_parameter_index;
            graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->vissObj.output0.image_handle[0];
        }
        if(obj->vissObj.params.enable_bayer_op)
        {
            status = add_graph_parameter_by_node_index(obj->graph, obj->vissObj.node, 6);
            obj->vissObj.output2.graph_parameter_index = graph_parameter_index;
            graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->vissObj.output2.image_handle[0];
        }
#else
        status = add_graph_parameter_by_node_index(obj->graph, obj->vissObj.node, 6);
        obj->vissObj.output2.graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->vissObj.output2.image_handle[0];
#endif
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
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

#if defined(SOC_AM62A)
    char * input_filename = "/opt/edgeai-tiovx-modules/data/input/ov2312_raw/rgbir/frame-1-000001.bin";
    char * output_filename = "/opt/edgeai-tiovx-modules/data/output/ov2312-rgbir-1600x1300-frame-1-000001.nv12";
#else
    char * input_filename = "/opt/edgeai-tiovx-modules/data/input/imx219_1920x1080_capture.raw";
    char * output_filename = "/opt/edgeai-tiovx-modules/data/output/imx219_1920x1080_capture_nv12.yuv";
#endif

    tivx_raw_image input_o;
    vx_image output0_o, output2_o;
    vx_user_data_object aewb_o;
    vx_user_data_object h3a_o;

    TIOVXVISSModuleObj *vissObj = &obj->vissObj;
    vx_int32 bufq;
    uint32_t num_refs;
    int32_t frame_count;

    void *inAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    void *out0Addr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    void *out2Addr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    void *aewbAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    void *h3aAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};

    vx_uint32 inSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32 out0Sizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32 out2Sizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32 aewbSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32 h3aSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];

    allocate_raw_image_buffers(&vissObj->input, inAddr, inSizes);

#if defined(SOC_AM62A)
    if(vissObj->params.enable_ir_op)
        allocate_image_buffers(&vissObj->output0, out0Addr, out0Sizes);
    
    if(vissObj->params.enable_bayer_op)
        allocate_image_buffers(&vissObj->output2, out2Addr, out2Sizes);
#else
    allocate_image_buffers(&vissObj->output2, out2Addr, out2Sizes);
#endif

    allocate_user_data_buffers(vissObj->ae_awb_result_arr, aewbAddr, aewbSizes, APP_BUFQ_DEPTH);
    allocate_user_data_buffers(vissObj->h3a_stats_arr, h3aAddr, h3aSizes, APP_BUFQ_DEPTH);

    bufq = 0;

    assign_raw_image_buffers(&vissObj->input, inAddr[bufq], inSizes[bufq], bufq);

#if defined(SOC_AM62A)
    if(vissObj->params.enable_ir_op)
        assign_image_buffers(&vissObj->output0, out0Addr[bufq], out0Sizes[bufq], bufq);
    if(vissObj->params.enable_bayer_op)
        assign_image_buffers(&vissObj->output2, out2Addr[bufq], out2Sizes[bufq], bufq);
#else
    assign_image_buffers(&vissObj->output2, out2Addr[bufq], out2Sizes[bufq], bufq);
#endif

    assign_user_data_buffers(vissObj->ae_awb_result_arr, aewbAddr[bufq], aewbSizes[bufq], bufq);
    assign_user_data_buffers(vissObj->h3a_stats_arr, h3aAddr[bufq], h3aSizes[bufq], bufq);

    frame_count = 0;
    while (frame_count < NUM_ITERATIONS)
    {
        APP_ERROR("Running frame %d\n", frame_count);
        readRawImage(input_filename, vissObj->input.image_handle[0]);

        APP_PRINTF("Enqueueing input raw buffers!\n");
        vxGraphParameterEnqueueReadyRef(obj->graph, vissObj->input.graph_parameter_index, (vx_reference*)&vissObj->input.image_handle[0], 1);

        APP_PRINTF("Enqueueing ae-awb buffers!\n");
        vxGraphParameterEnqueueReadyRef(obj->graph, vissObj->ae_awb_result_graph_parameter_index, (vx_reference*)&vissObj->ae_awb_result_handle[0], 1);

        APP_PRINTF("Enqueueing output image buffers!\n");

#if defined(SOC_AM62A)
        if(vissObj->params.enable_ir_op)
            vxGraphParameterEnqueueReadyRef(obj->graph, vissObj->output0.graph_parameter_index, (vx_reference*)&vissObj->output0.image_handle[0], 1);
        if(vissObj->params.enable_bayer_op)
            vxGraphParameterEnqueueReadyRef(obj->graph, vissObj->output2.graph_parameter_index, (vx_reference*)&vissObj->output2.image_handle[0], 1);
#else
        vxGraphParameterEnqueueReadyRef(obj->graph, vissObj->output2.graph_parameter_index, (vx_reference*)&vissObj->output2.image_handle[0], 1);
#endif

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

#if defined(SOC_AM62A)
        if(vissObj->params.enable_ir_op)
            vxGraphParameterDequeueDoneRef(obj->graph, vissObj->output0.graph_parameter_index, (vx_reference*)&output0_o, 1, &num_refs);
        if(vissObj->params.enable_bayer_op)
            vxGraphParameterDequeueDoneRef(obj->graph, vissObj->output2.graph_parameter_index, (vx_reference*)&output2_o, 1, &num_refs);
#else
        vxGraphParameterDequeueDoneRef(obj->graph, vissObj->output2.graph_parameter_index, (vx_reference*)&output2_o, 1, &num_refs);
#endif        

        vxGraphParameterDequeueDoneRef(obj->graph, vissObj->ae_awb_result_graph_parameter_index, (vx_reference*)&aewb_o, 1, &num_refs);
        vxGraphParameterDequeueDoneRef(obj->graph, vissObj->h3a_stats_graph_parameter_index, (vx_reference*)&h3a_o, 1, &num_refs);

#if defined(SOC_AM62A)
        if(vissObj->params.enable_ir_op)
            writeImage(output_filename, vissObj->output0.image_handle[0]);
        if(vissObj->params.enable_bayer_op)
            writeImage(output_filename, vissObj->output2.image_handle[0]);
#else
        writeImage(output_filename, vissObj->output2.image_handle[0]);
#endif
        //writeUserData(output_filename, vissObj->h3a_stats_handle[0]);

        if((VX_SUCCESS == vxGetStatus((vx_reference)aewb_o)) && (VX_SUCCESS == vxGetStatus((vx_reference)h3a_o)))
        {
            tivx_h3a_data_t *h3a_buf;
            vx_map_id h3a_buf_map_id;

            tivx_ae_awb_params_t *aewb_buf;
            vx_map_id aewb_buf_map_id;

            /* Below works only for single-channel */
            vxMapUserDataObject(h3a_o, 0, sizeof(tivx_h3a_data_t), &h3a_buf_map_id, (void **)&h3a_buf, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);
            vxMapUserDataObject(aewb_o, 0, sizeof(tivx_ae_awb_params_t), &aewb_buf_map_id, (void **)&aewb_buf, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

#if defined(SOC_AM62A)
            OV2312_GetExpPrgFxn(&obj->sensor_in_data.ae_dynPrms);
#else
            IMX219_GetExpPrgFxn(&obj->sensor_in_data.ae_dynPrms);
#endif

            TI_2A_wrapper_process(&obj->aewbObj, &obj->aewbConfig, h3a_buf, &obj->sensor_in_data, aewb_buf, &obj->sensor_out_data);

            vxUnmapUserDataObject(h3a_o, h3a_buf_map_id);
            vxUnmapUserDataObject(aewb_o, aewb_buf_map_id);
        }

        frame_count++;
    }

    release_raw_image_buffers(&vissObj->input, inAddr[bufq], inSizes[bufq], bufq);

#if defined(SOC_AM62A)
    if(vissObj->params.enable_ir_op)
        release_image_buffers(&vissObj->output0, out0Addr[bufq], out0Sizes[bufq], bufq);
    if(vissObj->params.enable_bayer_op)
        release_image_buffers(&vissObj->output2, out2Addr[bufq], out2Sizes[bufq], bufq);
#else
    release_image_buffers(&vissObj->output2, out2Addr[bufq], out2Sizes[bufq], bufq);
#endif

    release_user_data_buffers(vissObj->ae_awb_result_arr, aewbAddr[bufq], aewbSizes[bufq], bufq);
    release_user_data_buffers(vissObj->h3a_stats_arr, h3aAddr[bufq], h3aSizes[bufq], bufq);

    delete_raw_image_buffers(&vissObj->input, inAddr, inSizes);

#if defined(SOC_AM62A)
    if(vissObj->params.enable_ir_op)
        delete_image_buffers(&vissObj->output0, out0Addr, out0Sizes);
    if(vissObj->params.enable_bayer_op)
        delete_image_buffers(&vissObj->output2, out2Addr, out2Sizes);
#else
    delete_image_buffers(&vissObj->output2, out2Addr, out2Sizes);
#endif

    delete_user_data_buffers(vissObj->ae_awb_result_arr, aewbAddr, aewbSizes, bufq);
    delete_user_data_buffers(vissObj->h3a_stats_arr, h3aAddr, h3aSizes, bufq);

    return status;
}

#if defined(SOC_AM62A)
/* Typically this is obtained by querying the sensor */
static int32_t OV2312_GetExpPrgFxn(IssAeDynamicParams *p_ae_dynPrms)
{
    int32_t  status = 0;
    uint8_t count = 0;

    p_ae_dynPrms->targetBrightnessRange.min = 40;
    p_ae_dynPrms->targetBrightnessRange.max = 50;
    p_ae_dynPrms->targetBrightness = 45;
    p_ae_dynPrms->threshold = 1;
    p_ae_dynPrms->enableBlc = 1;
    p_ae_dynPrms->exposureTimeStepSize = 1;

    p_ae_dynPrms->exposureTimeRange[count].min = 1;
    p_ae_dynPrms->exposureTimeRange[count].max = 1404;
    p_ae_dynPrms->analogGainRange[count].min = 1;
    p_ae_dynPrms->analogGainRange[count].max = 511;
    p_ae_dynPrms->digitalGainRange[count].min = 1;
    p_ae_dynPrms->digitalGainRange[count].max = 4095;
    count++;

    p_ae_dynPrms->numAeDynParams = count;
    return (status);
}
#else
/* Typically this is obtained by querying the sensor */
static int32_t IMX219_GetExpPrgFxn(IssAeDynamicParams *p_ae_dynPrms)
{
    int32_t  status = 0;
    uint8_t count = 0;

    p_ae_dynPrms->targetBrightnessRange.min = 40;
    p_ae_dynPrms->targetBrightnessRange.max = 50;
    p_ae_dynPrms->targetBrightness = 45;
    p_ae_dynPrms->threshold = 1;
    p_ae_dynPrms->enableBlc = 1;
    p_ae_dynPrms->exposureTimeStepSize = 1;

    p_ae_dynPrms->exposureTimeRange[count].min = 100;
    p_ae_dynPrms->exposureTimeRange[count].max = 33333;
    p_ae_dynPrms->analogGainRange[count].min = 1024;
    p_ae_dynPrms->analogGainRange[count].max = 8192;
    p_ae_dynPrms->digitalGainRange[count].min = 256;
    p_ae_dynPrms->digitalGainRange[count].max = 256;
    count++;

    p_ae_dynPrms->numAeDynParams = count;
    return (status);
}
#endif
