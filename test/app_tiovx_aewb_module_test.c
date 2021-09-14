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
#include "tiovx_aewb_module.h"
#include "tiovx_sensor_module.h"

#define APP_BUFQ_DEPTH   (1)

#define APP_NUM_SEN       (1)

#define IMAGE_WIDTH  (640)
#define IMAGE_HEIGHT (480)

typedef struct {

    /* OpenVX references */
    vx_context context;

    vx_graph   graph;

    TIOVXAEWBModuleObj  aewbObj;

} AppObj;

AppObj gAppObj;

static vx_status app_init(AppObj *obj);
static void app_deinit(AppObj *obj);
static vx_status app_create_graph(AppObj *obj);
static vx_status app_verify_graph(AppObj *obj);
static vx_status app_run_graph(AppObj *obj);
static void app_delete_graph(AppObj *obj);

vx_status app_modules_aewb_test(vx_int32 argc, vx_char* argv[])
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
        tivxImagingLoadKernels(obj->context);
    }

    if(status == VX_SUCCESS)
    {
        TIOVXAEWBModuleObj *aewbObj = &obj->aewbObj;

        aewbObj->in_bufq_depth = APP_BUFQ_DEPTH;
        aewbObj->out_bufq_depth = APP_BUFQ_DEPTH;

        SensorObj *sensorObj = &aewbObj->sensorObj;
        tiovx_querry_sensor(sensorObj);
        tiovx_init_sensor(sensorObj,"IMX390-UB953_D3");

        /* Initialize modules */
        status = tiovx_aewb_module_init(obj->context, aewbObj);
        APP_PRINTF("AEWB Init Done! \n");
    }

    return status;
}

static void app_deinit(AppObj *obj)
{        
    tiovx_deinit_sensor(&obj->aewbObj.sensorObj);

    tiovx_aewb_module_deinit(&obj->aewbObj);

    tivxImagingUnLoadKernels(obj->context);

    vxReleaseContext(&obj->context);
}

static void app_delete_graph(AppObj *obj)
{
    tiovx_aewb_module_delete(&obj->aewbObj);

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
        status = tiovx_aewb_module_create(obj->graph, &obj->aewbObj, NULL, TIVX_TARGET_IPU1_0);
    }

    graph_parameter_index = 0;
    if((vx_status)VX_SUCCESS == status)
    {       

        status = add_graph_parameter_by_node_index(obj->graph, obj->aewbObj.node, 2);

        obj->aewbObj.input_graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->aewbObj.aewb_input_handle[0];
        graph_parameter_index++;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = add_graph_parameter_by_node_index(obj->graph, obj->aewbObj.node, 4);

        obj->aewbObj.output_graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->aewbObj.aewb_output_handle[0];
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
        status = tiovx_aewb_module_release_buffers(&obj->aewbObj);
    }

    return status;
}

///////////////////////
vx_status allocate_single_user_data_buffer(vx_user_data_object user_data, void *virtAddr[], vx_uint32 sizes[])
{
    vx_status status = VX_SUCCESS;

    void      *buf_addr[TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};

    vx_size data_size;
    
    status = vxQueryUserDataObject(user_data, VX_USER_DATA_OBJECT_SIZE, &data_size, sizeof(data_size));

    if((vx_status)VX_SUCCESS == status)
    {
        void *pBase = tivxMemAlloc(data_size, TIVX_MEM_EXTERNAL);
        virtAddr[0] = (void *)pBase;
        sizes[0] = data_size;
    }

    return status;
}

vx_status delete_single_user_data_buffer(vx_user_data_object user_data, void *virtAddr[], vx_uint32 sizes[])
{
    vx_status status = VX_SUCCESS;

    vx_size data_size;

    status = vxQueryUserDataObject(user_data, VX_USER_DATA_OBJECT_SIZE, &data_size, sizeof(data_size));

    if((vx_status)VX_SUCCESS == status)
    {

        tivxMemFree(virtAddr[0], data_size, TIVX_MEM_EXTERNAL);

        virtAddr[0] = NULL;
        sizes[0] = 0;
    }

    return status;
};

vx_status assign_single_user_data_buffer(vx_user_data_object user_data, void *virtAddr[], vx_uint32 sizes[], vx_uint32 num_bufs)
{
    vx_status status = VX_SUCCESS;

    if((vx_status)VX_SUCCESS == status)
    {
        void * addr[4];
        vx_int32 bufsize[4];
        vx_int32 p;

        for(p = 0; p < num_bufs; p++)
        {
            addr[p] = virtAddr[p];
            bufsize[p] = sizes[p];
        }

        status = tivxReferenceImportHandle((vx_reference)user_data,
                                        (const void **)addr,
                                        (const uint32_t *)bufsize,
                                        num_bufs);
    }

    return status;
};

vx_status release_single_user_data_buffer(vx_user_data_object user_data, void *virtAddr[], vx_uint32 sizes[], vx_uint32 num_bufs)
{
    vx_status status = VX_SUCCESS;

    if((vx_status)VX_SUCCESS == status)
    {
        void * addr[4];
        vx_int32 bufsize[4];
        vx_int32 p;

        for(p = 0; p < num_bufs; p++)
        {
            addr[p] = NULL;
            bufsize[p] = sizes[p];
        }

        /* Assign NULL handles to the OpenVx objects as it will avoid
            doing a tivxMemFree twice, once now and once during release */
        status = tivxReferenceImportHandle((vx_reference)user_data,
                                            (const void **)addr,
                                            (const uint32_t *)bufsize,
                                            num_bufs);
    }

    return status;
};

vx_status allocate_user_data_buffers(vx_object_array obj_arr[], void *virtAddr[][TIOVX_MODULES_MAX_REF_HANDLES], vx_uint32 sizes[][TIOVX_MODULES_MAX_REF_HANDLES], vx_int32 bufq_depth)
{
    vx_status status = VX_SUCCESS;

    vx_size num_ch;
    vx_int32 bufq, ch;

    APP_PRINTF("Allocating User Data Buffers \n");

    for(bufq = 0; bufq < bufq_depth; bufq++)
    {
        vxQueryObjectArray(obj_arr[bufq], VX_OBJECT_ARRAY_NUMITEMS, &num_ch, sizeof(vx_size));

        if((vx_status)VX_SUCCESS == status)
        {
            for(ch = 0; ch < num_ch; ch++)
            {
                vx_user_data_object user_data = (vx_user_data_object)vxGetObjectArrayItem(obj_arr[bufq], ch);

                if((vx_status)VX_SUCCESS == status)
                {
                    status = allocate_single_user_data_buffer
                            (
                                user_data,
                                &virtAddr[bufq][ch],
                                &sizes[bufq][ch]
                            );
                }

                vxReleaseUserDataObject(&user_data);

                APP_PRINTF("virtAddr[%d][%d] = 0x%016lx, size = %d\n", bufq, ch, (unsigned long int)virtAddr[bufq][ch], sizes[bufq][ch]);

                if((vx_status)VX_SUCCESS != status)
                {
                    APP_PRINTF("Unable to allocate single user data buffer!\n");
                    break;
                }
            }
        }
    }

    return status;
}

vx_status delete_user_data_buffers(vx_object_array obj_arr[], void *virtAddr[][TIOVX_MODULES_MAX_REF_HANDLES], vx_uint32 sizes[][TIOVX_MODULES_MAX_REF_HANDLES],  vx_int32 bufq_depth)
{
    vx_status status = VX_SUCCESS;

    vx_int32 bufq, ch;
    vx_size num_ch;

    APP_PRINTF("Deleting User Data Buffers \n");
    for(bufq = 0; bufq < bufq_depth; bufq++)
    {
        vxQueryObjectArray(obj_arr[bufq], VX_OBJECT_ARRAY_NUMITEMS, &num_ch, sizeof(vx_size));

        for(ch = 0; ch < num_ch; ch++)
        {
            vx_user_data_object user_data = (vx_user_data_object)vxGetObjectArrayItem(obj_arr[bufq], ch);

            APP_PRINTF("virtAddr[%d][%d] = 0x%016lx, size = %d\n", bufq, ch, (unsigned long int)virtAddr[bufq][ch], sizes[bufq][ch]);

            if((vx_status)VX_SUCCESS == status)
            {
                status = delete_single_user_data_buffer
                        (
                            user_data,
                            &virtAddr[bufq][ch],
                            &sizes[bufq][ch]
                        );

                if((vx_status)VX_SUCCESS != status)
                {
                    APP_PRINTF("Unable to delete single user data buffer!\n");
                    break;
                }
            }

            vxReleaseUserDataObject(&user_data);
        }
    }

    return status;
}

vx_status assign_user_data_buffers(vx_object_array obj_arr[], void *virtAddr[], vx_uint32 sizes[], vx_int32 bufq)
{
    vx_status status = VX_SUCCESS;

    vx_int32 ch;
    vx_size num_ch;

    vxQueryObjectArray(obj_arr[bufq], VX_OBJECT_ARRAY_NUMITEMS, &num_ch, sizeof(vx_size));

    APP_PRINTF("Assigning User Data Buffers \n");

    for(ch = 0; ch < num_ch; ch++)
    {
        vx_user_data_object user_data = (vx_user_data_object)vxGetObjectArrayItem(obj_arr[bufq], ch);

        APP_PRINTF("virtAddr[%d][%d] = 0x%016lx, size = %d\n", bufq, ch, (unsigned long int)virtAddr[ch], sizes[ch]);

        if((vx_status)VX_SUCCESS == status)
        {
            status = assign_single_user_data_buffer(user_data, &virtAddr[ch], &sizes[ch], 1);
        }

        if((vx_status)VX_SUCCESS != status)
        {
            APP_PRINTF("Unable to assign single user data buffer!\n");
            break;
        }

        vxReleaseUserDataObject(&user_data);
    }

    return status;
}

vx_status release_user_data_buffers(vx_object_array obj_arr[], void *virtAddr[], vx_uint32 sizes[], vx_int32 bufq)
{
    vx_status status = VX_SUCCESS;

    vx_int32 ch, ctr;
    vx_size num_ch;

    vxQueryObjectArray(obj_arr[bufq], VX_OBJECT_ARRAY_NUMITEMS, &num_ch, sizeof(vx_size));

    APP_PRINTF("Releasing User Data Buffers \n");

    ctr = 0;
    for(ch = 0; ch < num_ch; ch++)
    {
        vx_user_data_object user_data = (vx_user_data_object)vxGetObjectArrayItem(obj_arr[bufq], ch);

        APP_PRINTF("virtAddr[%d][%d] = 0x%016lx, size = %d\n", bufq, ch, (unsigned long int)virtAddr[ch], sizes[ch]);

        if((vx_status)VX_SUCCESS == status)
        {
            status = release_single_user_data_buffer(user_data, &virtAddr[ctr], &sizes[ctr], 1);
        }

        if((vx_status)VX_SUCCESS != status)
        {
            APP_PRINTF("Unable to assign single user data buffer!\n");
            break;
        }

        vxReleaseUserDataObject(&user_data);
    }

    return status;
}
///////////////

static vx_status app_run_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    TIOVXAEWBModuleObj *aewbObj = &obj->aewbObj;
    vx_int32 bufq;

    void *inAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    void *outAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};

    vx_uint32 inSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32 outSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];

    allocate_user_data_buffers(aewbObj->aewb_input_arr, inAddr, inSizes, APP_BUFQ_DEPTH);
    allocate_user_data_buffers(aewbObj->aewb_output_arr, outAddr, outSizes, APP_BUFQ_DEPTH);

    bufq = 0;

    assign_user_data_buffers(aewbObj->aewb_input_arr, inAddr[bufq], inSizes[bufq], bufq);
    assign_user_data_buffers(aewbObj->aewb_output_arr, outAddr[bufq], outSizes[bufq], bufq);

    //status = vxProcessGraph(obj->graph);

    release_user_data_buffers(aewbObj->aewb_input_arr, inAddr[bufq], inSizes[bufq], bufq);
    release_user_data_buffers(aewbObj->aewb_output_arr, outAddr[bufq], outSizes[bufq], bufq);

    delete_user_data_buffers(aewbObj->aewb_input_arr, inAddr, inSizes, bufq);
    delete_user_data_buffers(aewbObj->aewb_output_arr, outAddr, outSizes, bufq);
    
    return status;
}

