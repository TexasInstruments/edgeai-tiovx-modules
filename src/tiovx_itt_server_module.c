#include "tiovx_itt_server_module.h"

#include <itt_server.h>
#include <itt_srvr_remote.h>
#include <app_iss.h>
#include <tiovx_utils.h>

static TIOVXITTServerModuleObj g_ITTobj;
uint8_t g_ITTinit = 0;
pthread_mutex_t lock;

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
    num_bytes_io = writeRawImage(raw_image_fname, obj->obj_viss->input.image_handle[0]);
    if(num_bytes_io < 0)
    {
        printf("Error writing to RAW file \n");
        return VX_FAILURE;
    }

    snprintf(yuv_image_fname, TIOVX_MODULES_MAX_FNAME, "%s/%s_%04d.yuv", test_data_path, "img_viss", file_index);
    printf("YUV file name %s \n", yuv_image_fname);
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
            break;
        case LDC:
            g_ITTobj.obj_ldc = obj;
            status = VX_SUCCESS;
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
    pthread_mutex_lock(&lock);

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

    pthread_mutex_unlock(&lock);
    
    return (status);
}
