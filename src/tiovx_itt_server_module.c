#include "tiovx_itt_server_module.h"

#include <itt_server.h>
#include <itt_srvr_remote.h>
#include <app_iss.h>

static TIOVXITTServerModuleObj g_ITTobj;
uint8_t g_ITTinit = 0;

/*
Checks if the plugin is relevant to VISS. Returns
    0, if False
    1, if True
*/
static uint8_t is_viss_plugin(uint32_t plugin_id)
{
    switch (plugin_id)
    {
        case DCC_ID_H3A_AEWB_CFG:
        case DCC_ID_H3A_MUX_LUTS:
        case DCC_ID_RFE_DECOMPAND:
        case DCC_ID_IPIPE_RGB_RGB_1:
        case DCC_ID_NSF4:
        case DCC_ID_BLACK_CLAMP:
        case DCC_ID_IPIPE_CFA:
        case DCC_ID_VISS_GLBCE:
        case DCC_ID_VISS_LSC:
        case DCC_ID_VISS_DPC:
        case DCC_ID_VISS_YEE:
        case DCC_ID_RAWFE_WB1_VS:
            return 1U;
            break;
        default:
            return 0;
    }
    return 0;
}

/*
Checks if the plugin is relevant to LDC. Returns
    0, if False
    1, if True
*/
static uint8_t is_ldc_plugin(uint32_t plugin_id)
{
    switch (plugin_id)
    {
        case DCC_ID_MESH_LDC_J7:
            return 1U;
            break;
        default:
            return 0;
    }
    return 0;
}

static char *app_get_test_file_path()
{
    char *tivxPlatformGetEnv(char *env_var);

    #if defined(SYSBIOS)
    return tivxPlatformGetEnv("VX_TEST_DATA_PATH");
    #else
    return getenv("VX_TEST_DATA_PATH");
    #endif
}

static vx_status writeImage(char* file_name, vx_image img)
{
    vx_status status;

    printf("In writeImage\n");

    status = vxGetStatus((vx_reference)img);

    printf("status of image: %d\n", status);

    if((vx_status)VX_SUCCESS == status)
    {
        FILE * fp = fopen(file_name,"wb");
        vx_int32  j;

        if(fp == NULL)
        {
            TIOVX_MODULE_ERROR("Unable to open file %s \n", file_name);
            return (VX_FAILURE);
        }

        {
            vx_rectangle_t rect;
            vx_imagepatch_addressing_t image_addr;
            vx_map_id map_id;
            void * data_ptr;
            vx_uint32  img_width;
            vx_uint32  img_height;
            vx_uint32  num_bytes = 0;
            vx_size    num_planes;
            vx_uint32  plane;
            vx_uint32  plane_size;
            vx_df_image img_format;

            vxQueryImage(img, VX_IMAGE_WIDTH, &img_width, sizeof(vx_uint32));
            vxQueryImage(img, VX_IMAGE_HEIGHT, &img_height, sizeof(vx_uint32));
            vxQueryImage(img, VX_IMAGE_PLANES, &num_planes, sizeof(vx_size));
            vxQueryImage(img, VX_IMAGE_FORMAT, &img_format, sizeof(vx_df_image));

            for (plane = 0; plane < num_planes; plane++)
            {
                rect.start_x = 0;
                rect.start_y = 0;
                rect.end_x = img_width;
                rect.end_y = img_height;
                status = vxMapImagePatch(img,
                                        &rect,
                                        plane,
                                        &map_id,
                                        &image_addr,
                                        &data_ptr,
                                        VX_READ_ONLY,
                                        VX_MEMORY_TYPE_HOST,
                                        VX_NOGAP_X);

                TIOVX_MODULE_PRINTF("image_addr.dim_x = %d\n ", image_addr.dim_x);
                TIOVX_MODULE_PRINTF("image_addr.dim_y = %d\n ", image_addr.dim_y);
                TIOVX_MODULE_PRINTF("image_addr.step_x = %d\n ", image_addr.step_x);
                TIOVX_MODULE_PRINTF("image_addr.step_y = %d\n ", image_addr.step_y);
                TIOVX_MODULE_PRINTF("image_addr.stride_y = %d\n ", image_addr.stride_y);
                TIOVX_MODULE_PRINTF("image_addr.stride_x = %d\n ", image_addr.stride_x);
                TIOVX_MODULE_PRINTF("\n");

                num_bytes = 0;
                for (j = 0; j < (image_addr.dim_y/image_addr.step_y); j++)
                {
                    num_bytes += image_addr.stride_x * fwrite(data_ptr, image_addr.stride_x, (image_addr.dim_x/image_addr.step_x), fp);
                    data_ptr += image_addr.stride_y;
                }

                plane_size = (image_addr.dim_y/image_addr.step_y) * ((image_addr.dim_x * image_addr.stride_x)/image_addr.step_x);

                if(num_bytes != plane_size)
                    TIOVX_MODULE_ERROR("Plane [%d] bytes written = %d, expected = %d\n", plane, num_bytes, plane_size);

                vxUnmapImagePatch(img, map_id);
            }

        }

        fclose(fp);
    }

    return(status);
}

static vx_status writeRawImage(char* file_name, tivx_raw_image image)
{
    vx_status status;

    printf("In writeRawImage\n");

    status = vxGetStatus((vx_reference)image);

    printf("status of image: %d\n", status);

    if((vx_status)VX_SUCCESS == status)
    {
        FILE * fp = fopen(file_name,"wb");
        vx_int32  j;

        printf("status of image: %d\n", status);

        if(fp == NULL)
        {
            TIOVX_MODULE_ERROR("Unable to open file %s \n", file_name);
            return (VX_FAILURE);
        }

        {
            vx_uint32 width, height;
            vx_imagepatch_addressing_t image_addr;
            vx_rectangle_t rect;
            vx_map_id map_id;
            void *data_ptr;
            vx_uint32 bpp = 1;
            vx_uint32 num_bytes;
            tivx_raw_image_format_t format[3];
            vx_int32 plane, num_planes, plane_size;
            vx_uint32 num_exposures;
            vx_bool line_interleaved = vx_false_e;

            tivxQueryRawImage(image, TIVX_RAW_IMAGE_WIDTH, &width, sizeof(vx_uint32));
            tivxQueryRawImage(image, TIVX_RAW_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
            tivxQueryRawImage(image, TIVX_RAW_IMAGE_FORMAT, &format, sizeof(format));
            tivxQueryRawImage(image, TIVX_RAW_IMAGE_NUM_EXPOSURES, &num_exposures, sizeof(num_exposures));
            tivxQueryRawImage(image, TIVX_RAW_IMAGE_LINE_INTERLEAVED, &line_interleaved, sizeof(line_interleaved));

            if(line_interleaved == vx_true_e)
            {
                num_planes = 1;
            }
            else
            {
                num_planes = num_exposures;
            }

            if( format[0].pixel_container == TIVX_RAW_IMAGE_16_BIT )
            {
                bpp = 2;
            }
            else if( format[0].pixel_container == TIVX_RAW_IMAGE_8_BIT )
            {
                bpp = 1;
            }
            else if( format[0].pixel_container == TIVX_RAW_IMAGE_P12_BIT )
            {
                bpp = 0;
            }

            rect.start_x = 0;
            rect.start_y = 0;
            rect.end_x = width;
            rect.end_y = height;

            for (plane = 0; plane < num_planes; plane++)
            {
                tivxMapRawImagePatch(image,
                    &rect,
                    plane,
                    &map_id,
                    &image_addr,
                    &data_ptr,
                    VX_READ_ONLY,
                    VX_MEMORY_TYPE_HOST,
                    TIVX_RAW_IMAGE_PIXEL_BUFFER
                    );

                uint8_t *pIn = (uint8_t *)data_ptr;
                num_bytes = 0;
                if(line_interleaved == vx_true_e)
                {
                    for (j = 0; j < (image_addr.dim_y * num_exposures); j++)
                    {
                        num_bytes += fwrite(pIn, 1, image_addr.dim_x * bpp, fp);
                        pIn += image_addr.stride_y;
                    }
                }
                else
                {
                    for (j = 0; j < image_addr.dim_y; j++)
                    {
                        num_bytes += fwrite(pIn, 1, image_addr.dim_x * bpp, fp);
                        pIn += image_addr.stride_y;
                    }
                }

                plane_size = (image_addr.dim_y * image_addr.dim_x* bpp);

                if(num_bytes != plane_size)
                    TIOVX_MODULE_ERROR("Plane [%d] bytes written = %d, expected = %d\n", plane, num_bytes, plane_size);

                tivxUnmapRawImagePatch(image, map_id);
            }
        }

        fclose(fp);
    }

    return(status);
}

int save_debug_images(TIOVXITTServerModuleObj *obj)
{
    int num_bytes_io = 0;
    static int file_index = 0;
    char raw_image_fname[TIOVX_MODULES_MAX_FNAME];
    char yuv_image_fname[TIOVX_MODULES_MAX_FNAME];
    char h3a_image_fname[TIOVX_MODULES_MAX_FNAME];
    char failsafe_test_data_path[3] = "./";
    char * test_data_path = app_get_test_file_path();
    struct stat s;

    if(NULL == test_data_path)
    {
        printf("Test data path is NULL. Defaulting to current folder \n");
        test_data_path = failsafe_test_data_path;
    }

    if (stat(test_data_path, &s))
    {
        printf("Test data path %s does not exist. Defaulting to current folder \n", test_data_path);
        test_data_path = failsafe_test_data_path;
    }


    snprintf(raw_image_fname, TIOVX_MODULES_MAX_FNAME, "%s/%s_%04d.raw", test_data_path, "img", file_index);
    printf("RAW file name %s \n", raw_image_fname);
    // num_bytes_io = write_output_image_raw(raw_image_fname, obj->raw);
    printf("obj->input.image_handle[0]: %p\n", obj->obj_viss->input.image_handle[0]);
    // num_bytes_io = writeRawImage(raw_image_fname, obj->input.image_handle[0]);
    num_bytes_io = writeRawImage(raw_image_fname, obj->obj_viss->input.image_handle[0]);
    if(num_bytes_io < 0)
    {
        printf("Error writing to RAW file \n");
        return VX_FAILURE;
    }

    snprintf(yuv_image_fname, TIOVX_MODULES_MAX_FNAME, "%s/%s_%04d.yuv", test_data_path, "img_viss", file_index);
    printf("YUV file name %s \n", yuv_image_fname);
    // num_bytes_io = write_output_image_nv12_8bit(yuv_image_fname, obj->y8_r8_c2);
    num_bytes_io = writeImage(yuv_image_fname, obj->obj_viss->output2.image_handle[0]);
    if(num_bytes_io < 0)
    {
        printf("Error writing to VISS NV12 file \n");
        return VX_FAILURE;
    }

    file_index++;
    return (file_index-1);
}

vx_status tiovx_itt_module_update_dcc(TIOVXVISSModuleObj *obj, uint8_t* dcc_buf, uint32_t dcc_buf_size)
{
    vx_status status = VX_SUCCESS;

    status = appUpdateVpacDcc(dcc_buf, dcc_buf_size, g_ITTobj.obj_viss->context,
                g_ITTobj.obj_viss->node /* node_viss */, 0,
                NULL /* node_aewb*/, 0,
                NULL /* node_ldc */, 0
             );

    return status;
}

vx_status tiovx_itt_register_object(void *obj, uint8_t object_name) {

    vx_status status = VX_SUCCESS;

    switch(object_name) {
        case VISS:
            g_ITTobj.obj_viss = obj;
            status = VX_SUCCESS;
            printf("VISS OBJECT REGISTERD\n");
            break;
        case LDC:
            g_ITTobj.obj_ldc = obj;
            status = VX_SUCCESS;
            printf("LDC OBJECT REGISTERD\n");
            break;
        default:
            status = VX_FAILURE;
    }

    return status;
}

vx_status tiovx_itt_handle_dcc(TIOVXITTServerModuleObj *obj, uint8_t* dcc_buf, uint32_t dcc_buf_size) {
    vx_status status = VX_SUCCESS;

    dcc_component_header_type *header_data;
    header_data = (dcc_component_header_type*)dcc_buf;

    if(1U == is_viss_plugin(header_data->dcc_descriptor_id)) {

        if(g_ITTobj.obj_viss == NULL) {
            printf("VISS Object is NULL!!\n");
        }
        else {
            /* Trigger VISS udpate dcc callback */
            status = appUpdateVpacDcc(dcc_buf, dcc_buf_size, g_ITTobj.obj_viss->context,
                g_ITTobj.obj_viss->node /* node_viss */, 0,
                NULL /* node_aewb*/, 0,
                NULL /* node_ldc */, 0
             );
        }
    }
    else if(1U == is_ldc_plugin(header_data->dcc_descriptor_id)) {

        if(g_ITTobj.obj_ldc == NULL) {
            printf("LDC Object is NULL!!\n");
        }
        else {
            /* Trigger LDC update dcc callback */
            status = appUpdateVpacDcc(dcc_buf, dcc_buf_size, g_ITTobj.obj_ldc->context,
                NULL /* node_viss */, 0,
                NULL /* node_aewb*/, 0,
                g_ITTobj.obj_ldc->node /* node_ldc */, 0
             );
        }

    }

    return status;
}

vx_status tiovx_itt_server_module_init()
{
    /* protected (with ref count with lock) not guaranteed ldc will be called */
    vx_status status = VX_SUCCESS;

    if(!g_ITTinit) {

        printf("\tITT SERVER INITIALIZED!!!!\n");
        
        /* Initialize ITT Server after graph is created. */
        if((vx_status)VX_SUCCESS == status)
        {
            /* Initiates ITT server thread on A72/Linux */
            status = itt_server_init((void*)&g_ITTobj, (void*)save_debug_images, (void*)tiovx_itt_handle_dcc);
            if(status != 0)
            {
                printf("Warning : Failed to initialize ITT server. Live tuning will not work \n");
            }
        }

        if((vx_status)VX_SUCCESS == status)
        {
            /* Initiates remote server for 2A control. Ported from RTOS */
            status = IttRemoteServer_Init();
            if(status!=0)
            {
                printf("Warning: Failed to create remote ITT server failed. Live tuning will not work !!!\n");
            }
        }
        
        g_ITTinit = 1;
    }

    return (status);
}
