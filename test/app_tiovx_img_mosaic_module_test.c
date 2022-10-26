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
#include "tiovx_img_mosaic_module.h"

#define APP_BUFQ_DEPTH   (2)
#define APP_NUM_CH       (2)
#define APP_NUM_OUTPUTS  (2)

#define IMAGE_WIDTH  (1280)
#define IMAGE_HEIGHT (720)

typedef struct {

    /* OpenVX references */
    vx_context context;

    vx_graph   graph;

    TIOVXImgMosaicModuleObj  imgMosaicObj;

} AppObj;

static AppObj gAppObj;

static vx_status app_init(AppObj *obj);
static void app_deinit(AppObj *obj);
static vx_status app_create_graph(AppObj *obj);
static vx_status app_verify_graph(AppObj *obj);
static vx_status app_run_graph(AppObj *obj);
static void app_delete_graph(AppObj *obj);

vx_status app_modules_img_mosaic_test(vx_int32 argc, vx_char* argv[])
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
        TIOVXImgMosaicModuleObj *imgMosaicObj = &obj->imgMosaicObj;

        imgMosaicObj->out_width    = DISPLAY_WIDTH;
        imgMosaicObj->out_height   = DISPLAY_HEIGHT;
        imgMosaicObj->out_bufq_depth = APP_BUFQ_DEPTH;
        imgMosaicObj->color_format = VX_DF_IMAGE_NV12;

        imgMosaicObj->num_channels = 1;
        imgMosaicObj->num_inputs   = 1;
        imgMosaicObj->inputs[0].width = IMAGE_WIDTH;
        imgMosaicObj->inputs[0].height = IMAGE_HEIGHT;
        imgMosaicObj->inputs[0].color_format = VX_DF_IMAGE_NV12;
        imgMosaicObj->inputs[0].bufq_depth = APP_BUFQ_DEPTH;

        tivxImgMosaicParamsSetDefaults(&imgMosaicObj->params);

        imgMosaicObj->params.num_windows  = 1;
        imgMosaicObj->params.windows[0].startX  = 500;
        imgMosaicObj->params.windows[0].startY  = 200;
        imgMosaicObj->params.windows[0].width   = IMAGE_WIDTH;
        imgMosaicObj->params.windows[0].height  = IMAGE_HEIGHT;
        imgMosaicObj->params.windows[0].input_select   = 0;
        imgMosaicObj->params.windows[0].channel_select = 0;

        /* Number of time to clear the output buffer before it gets reused */
        imgMosaicObj->params.clear_count  = 4;

        /* Initialize modules */
        status = tiovx_img_mosaic_module_init(obj->context, imgMosaicObj);
        APP_PRINTF("Image Mosaic Init Done! \n");
    }

    return status;
}

static void app_deinit(AppObj *obj)
{
    tiovx_img_mosaic_module_deinit(&obj->imgMosaicObj);

    vxReleaseContext(&obj->context);
}

static void app_delete_graph(AppObj *obj)
{
    tiovx_img_mosaic_module_delete(&obj->imgMosaicObj);

    vxReleaseGraph(&obj->graph);
}

static vx_status app_create_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[8];
    vx_int32 graph_parameter_index;

    vx_object_array input_arr_user[] = {NULL};

    obj->graph = vxCreateGraph(obj->context);
    status = vxGetStatus((vx_reference)obj->graph);

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_img_mosaic_module_create(obj->graph, &obj->imgMosaicObj, obj->imgMosaicObj.background_image[0], input_arr_user, TIVX_TARGET_VPAC_MSC1);
    }

    graph_parameter_index = 0;
    if((vx_status)VX_SUCCESS == status)
    {   /* Output image index is 1 */
        status = add_graph_parameter_by_node_index(obj->graph, obj->imgMosaicObj.node, 1);
        obj->imgMosaicObj.output_graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->imgMosaicObj.output_image[0];
        graph_parameter_index++;
    }

    if((vx_status)VX_SUCCESS == status)
    {   /* Background image index is 2 */
        status = add_graph_parameter_by_node_index(obj->graph, obj->imgMosaicObj.node, 2);
        obj->imgMosaicObj.background_graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->imgMosaicObj.background_image[0];
        graph_parameter_index++;
    }

    if((vx_status)VX_SUCCESS == status)
    {   /* Inputs start from image index 3 */
        status = add_graph_parameter_by_node_index(obj->graph, obj->imgMosaicObj.node, 3);
        obj->imgMosaicObj.inputs[0].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->imgMosaicObj.inputs[0].image_handle[0];
        graph_parameter_index++;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = vxSetGraphScheduleConfig(obj->graph,
                    VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
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
        status = tiovx_img_mosaic_module_release_buffers(&obj->imgMosaicObj);
    }

    return status;
}

static vx_status app_run_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    TIOVXImgMosaicModuleObj *imgMosaicObj = &obj->imgMosaicObj;
    vx_int32 bufq;
    vx_size out_num_planes;
    vx_size back_num_planes;

    void *inAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    void *outAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    void *backAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};

    vx_uint32 inSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32 outSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32 backSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];

    vxQueryImage(imgMosaicObj->output_image[0], VX_IMAGE_PLANES, &out_num_planes, sizeof(out_num_planes));
    vxQueryImage(imgMosaicObj->background_image[0], VX_IMAGE_PLANES, &back_num_planes, sizeof(back_num_planes));

    /* These can be moved to app_init() */
    allocate_image_buffers(&imgMosaicObj->inputs[0], inAddr, inSizes);
    for(bufq = 0; bufq < APP_BUFQ_DEPTH; bufq++)
    {
        allocate_single_image_buffer(imgMosaicObj->output_image[bufq], outAddr[bufq], outSizes[bufq]);
    }
    for(bufq = 0; bufq < APP_BUFQ_DEPTH; bufq++)
    {
        allocate_single_image_buffer(imgMosaicObj->background_image[bufq], backAddr[bufq], backSizes[bufq]);
    }

    bufq = 0;
    assign_image_buffers(&imgMosaicObj->inputs[0], inAddr[bufq], inSizes[bufq], bufq);
    assign_single_image_buffer(imgMosaicObj->output_image[0], outAddr[bufq], outSizes[bufq], out_num_planes);
    assign_single_image_buffer(imgMosaicObj->background_image[0], outAddr[bufq], outSizes[bufq], back_num_planes);

    //status = vxProcessGraph(obj->graph);

    release_image_buffers(&imgMosaicObj->inputs[0], inAddr[bufq], inSizes[bufq], bufq);
    release_single_image_buffer(imgMosaicObj->output_image[0], outAddr[bufq], outSizes[bufq], out_num_planes);
    release_single_image_buffer(imgMosaicObj->background_image[0], outAddr[bufq], outSizes[bufq], back_num_planes);

    /* These can move to deinit() */
    delete_image_buffers(&imgMosaicObj->inputs[0], inAddr, inSizes);
    for(bufq = 0; bufq < APP_BUFQ_DEPTH; bufq++)
    {
        delete_single_image_buffer(imgMosaicObj->output_image[bufq], outAddr[bufq], outSizes[bufq]);
    }
    for(bufq = 0; bufq < APP_BUFQ_DEPTH; bufq++)
    {
        delete_single_image_buffer(imgMosaicObj->background_image[bufq], backAddr[bufq], backSizes[bufq]);
    }

    return status;
}
