#ifndef _TIOVX_ITT_SERVER_MODULE
#define _TIOVX_ITT_SERVER_MODULE

#include "tiovx_modules_common.h"
#include "tiovx_ldc_module.h"
#include "tiovx_viss_module.h"

/**
 *  \brief RPC Commands application can send to AEWB
 * \ingroup group_vision_function_imaging
 */

// typedef enum {
//     VISS = 0,
//     AEWB,
//     LDC,
// }TIOVX_NODES;

typedef vx_status (*DCCUpdateCallback_t)(void *obj, uint8_t *dcc_buf, uint32_t dcc_buf_size);

/** \brief VISS Module Data Structure
 *
 * Contains the data objects required to use tivxITTServerNode
 *
 */
typedef struct itt_server_obj {

    /*! VISS module object */
    TIOVXVISSModuleObj *obj_viss;

    /*! LDC module object */
    TIOVXLDCModuleObj *obj_ldc; 

}TIOVXITTServerModuleObj;
 
vx_status tiovx_itt_register_object(void *obj, uint8_t object_name);

vx_status tiovx_itt_handle_dcc(TIOVXITTServerModuleObj *obj, uint8_t* dcc_buf, uint32_t dcc_buf_size);

vx_status tiovx_itt_server_module_init();

#endif