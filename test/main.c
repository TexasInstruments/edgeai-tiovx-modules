/*
 *
 * Copyright (c) 2017 Texas Instruments Incorporated
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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <TI/tivx.h>
#include <app_init.h>

#define APP_MODULES_TEST_MULTI_SCALER (1)
#define APP_MODULES_TEST_COLOR_CONVERT (1)
#define APP_MODULES_TEST_IMG_MOSAIC (1)
#define APP_MODULES_TEST_PRE_PROC (1)
#define APP_MODULES_TEST_COLOR_BLEND (1)
#define APP_MODULES_TEST_LDC (1)
#define APP_MODULES_TEST_VISS (1)
#define APP_MODULES_TEST_PYRAMID (1)
#define APP_MODULES_TEST_DOF (1)

int32_t appInit()
{
    int32_t status = 0;

    status = appCommonInit();

    if(status==0)
    {
        tivxInit();
        tivxHostInit();
    }
    return status;
}

int32_t appDeInit()
{
    int32_t status = 0;

    tivxHostDeInit();
    tivxDeInit();
    appCommonDeInit();

    return status;
}

int main(int argc, char *argv[])
{
    int status = 0;

    status = appInit();

#if (APP_MODULES_TEST_MULTI_SCALER)
    if(status==0)
    {
        printf("Running multi-scaler module test\n");
        int app_modules_scaler_test(int argc, char* argv[]);

        status = app_modules_scaler_test(argc, argv);
    }
#endif

#if (APP_MODULES_TEST_COLOR_CONVERT)
    if(status==0)
    {
        printf("Running color convert module test\n");
        int app_modules_color_convert_test(int argc, char* argv[]);

        status = app_modules_color_convert_test(argc, argv);
    }
#endif

#if (APP_MODULES_TEST_IMG_MOSAIC)
    if(status==0)
    {
        printf("Running image mosaic module test\n");
        int app_modules_img_mosaic_test(int argc, char* argv[]);

        status = app_modules_img_mosaic_test(argc, argv);
    }
#endif

#if (APP_MODULES_TEST_PRE_PROC)
    if(status==0)
    {
        printf("Running DL pre-proc module test\n");
        int app_modules_dl_pre_proc_test(int argc, char* argv[]);

        status = app_modules_dl_pre_proc_test(argc, argv);
    }
#endif

#if (APP_MODULES_TEST_COLOR_BLEND)
    if(status==0)
    {
        printf("Running DL color-blend module test\n");
        int app_modules_dl_color_blend_test(int argc, char* argv[]);

        status = app_modules_dl_color_blend_test(argc, argv);
    }
#endif

#if (APP_MODULES_TEST_LDC)
    if(status==0)
    {
        printf("Running LDC module test\n");
        int app_modules_ldc_test(int argc, char* argv[]);

        status = app_modules_ldc_test(argc, argv);
    }
#endif

#if (APP_MODULES_TEST_VISS)
    if(status==0)
    {
        printf("Running VISS module test\n");
        int app_modules_viss_test(int argc, char* argv[]);

        status = app_modules_viss_test(argc, argv);
    }
#endif

#if (APP_MODULES_TEST_PYRAMID)
    if(status==0)
    {
        printf("Running PYRAMID module test\n");
        int app_modules_pyramid_test(int argc, char* argv[]);

        status = app_modules_pyramid_test(argc, argv);
    }
#endif

#if (APP_MODULES_TEST_DOF)
    if(status==0)
    {
        printf("Running DOF module test\n");
        int app_modules_dof_test(int argc, char* argv[]);

        status = app_modules_dof_test(argc, argv);
    }
#endif

    printf("All tests complete!\n");

    appDeInit();

    return status;
}
