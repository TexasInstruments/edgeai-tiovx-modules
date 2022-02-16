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

static vx_uint32 get_tensor_bitdepth(vx_enum tensor_type);

/*
 * Utility API used to add a graph parameter from a node, node parameter index
 */
vx_status add_graph_parameter_by_node_index(vx_graph graph, vx_node node, vx_uint32 node_parameter_index)
{
    vx_parameter parameter = vxGetParameterByIndex(node, node_parameter_index);
    vx_status status;
    status = vxAddParameterToGraph(graph, parameter);
    if(status == VX_SUCCESS)
    {
        status = vxReleaseParameter(&parameter);
    }
    return status;
}

vx_status allocate_single_raw_image_buffer(tivx_raw_image image, void *virtAddr[], vx_uint32 sizes[])
{
    vx_status status = VX_SUCCESS;

    void      *plane_addr[TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    vx_uint32  plane_sizes[TIOVX_MODULES_MAX_REF_HANDLES];

    vx_uint32 num_planes;

    if((vx_status)VX_SUCCESS == status)
    {
        /* Export handles to get valid size information. */
        status = tivxReferenceExportHandle((vx_reference)image,
                                            plane_addr,
                                            plane_sizes,
                                            TIOVX_MODULES_MAX_REF_HANDLES,
                                            &num_planes);

        if((vx_status)VX_SUCCESS == status)
        {
            vx_int32 p;
            for(p = 0; p < num_planes; p++)
            {
                virtAddr[p] = tivxMemAlloc(plane_sizes[p], TIVX_MEM_EXTERNAL);
                sizes[p] = plane_sizes[p];
            }
        }
    }

    return status;
}

vx_status delete_single_raw_image_buffer(tivx_raw_image image, void *virtAddr[], vx_uint32 sizes[])
{
    vx_status status = VX_SUCCESS;

    void      *plane_addr[TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    vx_uint32  plane_sizes[TIOVX_MODULES_MAX_REF_HANDLES];

    vx_uint32 num_planes;

    if((vx_status)VX_SUCCESS == status)
    {
        /* Export handles to get valid size information. */
        status = tivxReferenceExportHandle((vx_reference)image,
                                            plane_addr,
                                            plane_sizes,
                                            TIOVX_MODULES_MAX_REF_HANDLES,
                                            &num_planes);

        if((vx_status)VX_SUCCESS == status)
        {
            vx_int32 p;
            for(p = 0; p < num_planes; p++)
            {
                tivxMemFree(virtAddr[p], plane_sizes[p], TIVX_MEM_EXTERNAL);
                virtAddr[p] = NULL;
                sizes[p] = 0;
            }
        }
    }

    return status;
}

vx_status assign_single_raw_image_buffer(tivx_raw_image image, void *virtAddr[], vx_uint32 sizes[], vx_uint32 num_planes)
{
    vx_status status = VX_SUCCESS;

    if((vx_status)VX_SUCCESS == status)
    {
        void * addr[4];
        vx_int32 bufsize[4];
        vx_int32 p;

        for(p = 0; p < num_planes; p++)
        {
            addr[p] = virtAddr[p];
            bufsize[p] = sizes[p];
        }

        status = tivxReferenceImportHandle((vx_reference)image,
                                        (const void **)addr,
                                        (const uint32_t *)bufsize,
                                        num_planes);
    }

    return status;
}

vx_status release_single_raw_image_buffer(tivx_raw_image image, void *virtAddr[], vx_uint32 sizes[], vx_uint32 num_planes)
{
    vx_status status = VX_SUCCESS;

    if((vx_status)VX_SUCCESS == status)
    {
        void * addr[4];
        vx_int32 bufsize[4];
        vx_int32 p;

        for(p = 0; p < num_planes; p++)
        {
            addr[p] = NULL;
            bufsize[p] = sizes[p];
        }

        /* Assign NULL handles to the OpenVx objects as it will avoid
            doing a tivxMemFree twice, once now and once during release */
        status = tivxReferenceImportHandle((vx_reference)image,
                                            (const void **)addr,
                                            (const uint32_t *)bufsize,
                                            num_planes);
    }

    return status;
}

vx_status allocate_raw_image_buffers(RawImgObj *imgObj, void *virtAddr[][TIOVX_MODULES_MAX_REF_HANDLES], vx_uint32 sizes[][TIOVX_MODULES_MAX_REF_HANDLES])
{
    vx_status status = VX_SUCCESS;

    vx_int32 bufq, ch, ctr;
    vx_size num_ch;
    vx_uint32 num_exposures;
    vx_bool line_interleaved = vx_false_e;

    APP_PRINTF("Allocating Buffers \n");

    for(bufq = 0; bufq < imgObj->bufq_depth; bufq++)
    {
        vxQueryObjectArray(imgObj->arr[bufq], VX_OBJECT_ARRAY_NUMITEMS, &num_ch, sizeof(vx_size));

        if((vx_status)VX_SUCCESS == status)
        {
            vx_int32   l;

            ctr = 0;
            for(ch = 0; ch < num_ch; ch++)
            {
                tivx_raw_image image = (tivx_raw_image)vxGetObjectArrayItem(imgObj->arr[bufq], ch);

                tivxQueryRawImage(image, TIVX_RAW_IMAGE_NUM_EXPOSURES, &num_exposures, sizeof(num_exposures));
                tivxQueryRawImage(image, TIVX_RAW_IMAGE_LINE_INTERLEAVED, &line_interleaved, sizeof(line_interleaved));

                if(line_interleaved == vx_true_e)
                    num_exposures = 1;

                status = allocate_single_raw_image_buffer
                        (
                            image,
                            &virtAddr[bufq][ctr],
                            &sizes[bufq][ctr]
                        );

                for(l = 0; l < num_exposures; l++)
                {
                    APP_PRINTF("virtAddr[%d][%d] = 0x%016lx, size = %d\n", bufq, l, (unsigned long int)virtAddr[bufq][ctr + l], sizes[bufq][ctr + l]);
                }

                ctr += num_exposures;

                tivxReleaseRawImage(&image);

                if((vx_status)VX_SUCCESS != status)
                {
                    APP_PRINTF("Unable to allocate single raw image buffer!\n");
                    break;
                }
            }
        }
    }

    return status;
}

vx_status delete_raw_image_buffers(RawImgObj *imgObj, void *virtAddr[][TIOVX_MODULES_MAX_REF_HANDLES], vx_uint32 sizes[][TIOVX_MODULES_MAX_REF_HANDLES])
{
    vx_status status = VX_SUCCESS;

    vx_int32 bufq, ch, ctr;
    vx_size num_ch;
    vx_uint32 num_exposures;
    vx_bool line_interleaved = vx_false_e;

    APP_PRINTF("Deleting Buffers \n");

    for(bufq = 0; bufq < imgObj->bufq_depth; bufq++)
    {
        vx_int32 l;
        vxQueryObjectArray(imgObj->arr[bufq], VX_OBJECT_ARRAY_NUMITEMS, &num_ch, sizeof(vx_size));

        ctr = 0;
        for(ch = 0; ch < num_ch; ch++)
        {
            tivx_raw_image image = (tivx_raw_image)vxGetObjectArrayItem(imgObj->arr[bufq], ch);

            tivxQueryRawImage(image, TIVX_RAW_IMAGE_NUM_EXPOSURES, &num_exposures, sizeof(num_exposures));
            tivxQueryRawImage(image, TIVX_RAW_IMAGE_LINE_INTERLEAVED, &line_interleaved, sizeof(line_interleaved));

            if(line_interleaved == vx_true_e)
                num_exposures = 1;

            for(l = 0; l < num_exposures; l++)
            {
                APP_PRINTF("virtAddr[%d][%d] = 0x%016lx, size = %d\n", bufq, l, (unsigned long int)virtAddr[bufq][ctr + l], sizes[bufq][ctr + l]);
            }

            status = delete_single_raw_image_buffer
                    (
                        image,
                        &virtAddr[bufq][ctr],
                        &sizes[bufq][ctr]
                    );

            if((vx_status)VX_SUCCESS != status)
            {
                APP_PRINTF("Unable to delete single image buffer!\n");
                break;
            }

            ctr += num_exposures;
            tivxReleaseRawImage(&image);
        }
    }

    return status;
}

vx_status assign_raw_image_buffers(RawImgObj *imgObj, void *virtAddr[], vx_uint32 sizes[], vx_int32 bufq)
{
    vx_status status = VX_SUCCESS;

    vx_int32 ch, ctr, l;
    vx_size num_ch;

    vxQueryObjectArray(imgObj->arr[bufq], VX_OBJECT_ARRAY_NUMITEMS, &num_ch, sizeof(vx_size));

    APP_PRINTF("Assigning Buffers \n");

    ctr = 0;
    for(ch = 0; ch < num_ch; ch++)
    {
        vx_uint32 num_exposures;
        vx_bool line_interleaved = vx_false_e;

        tivx_raw_image image = (tivx_raw_image)vxGetObjectArrayItem(imgObj->arr[bufq], ch);

        tivxQueryRawImage(image, TIVX_RAW_IMAGE_NUM_EXPOSURES, &num_exposures, sizeof(num_exposures));
        tivxQueryRawImage(image, TIVX_RAW_IMAGE_LINE_INTERLEAVED, &line_interleaved, sizeof(line_interleaved));

        if(line_interleaved == vx_true_e)
            num_exposures = 1;

        for(l = 0; l < num_exposures; l++)
        {
            APP_PRINTF("virtAddr[%d][%d] = 0x%016lx, size = %d\n", bufq, l, (unsigned long int)virtAddr[ctr + l], sizes[ctr + l]);
        }

        if((vx_status)VX_SUCCESS == status)
        {
            status = assign_single_raw_image_buffer(image, &virtAddr[ctr], &sizes[ctr], num_exposures);
        }

        if((vx_status)VX_SUCCESS != status)
        {
            APP_PRINTF("Unable to assign single image buffer!\n");
            break;
        }

        ctr += num_exposures;
        tivxReleaseRawImage(&image);
    }

    return status;
}

vx_status release_raw_image_buffers(RawImgObj *imgObj, void *virtAddr[], vx_uint32 sizes[], vx_int32 bufq)
{
    vx_status status = VX_SUCCESS;

    vx_int32 ch, ctr, l;
    vx_size num_ch;
    vx_uint32 num_exposures;
    vx_bool line_interleaved = vx_false_e;

    vxQueryObjectArray(imgObj->arr[bufq], VX_OBJECT_ARRAY_NUMITEMS, &num_ch, sizeof(vx_size));

    APP_PRINTF("Releasing Buffers \n");

    ctr = 0;
    for(ch = 0; ch < num_ch; ch++)
    {
        tivx_raw_image image = (tivx_raw_image)vxGetObjectArrayItem(imgObj->arr[bufq], ch);
        tivxQueryRawImage(image, TIVX_RAW_IMAGE_NUM_EXPOSURES, &num_exposures, sizeof(num_exposures));
        tivxQueryRawImage(image, TIVX_RAW_IMAGE_LINE_INTERLEAVED, &line_interleaved, sizeof(line_interleaved));

        if(line_interleaved == vx_true_e)
            num_exposures = 1;

        for(l = 0; l < num_exposures; l++)
        {
            APP_PRINTF("virtAddr[%d][%d] = 0x%016lx, size = %d\n", bufq, l, (unsigned long int)virtAddr[ctr + l], sizes[ctr + l]);
        }

        if((vx_status)VX_SUCCESS == status)
        {
            status = release_single_raw_image_buffer(image, &virtAddr[ctr], &sizes[ctr], num_exposures);
        }

        if((vx_status)VX_SUCCESS != status)
        {
            APP_PRINTF("Unable to assign single image buffer!\n");
            break;
        }

        ctr += num_exposures;
        tivxReleaseRawImage(&image);
    }

    return status;
}

vx_status allocate_single_image_buffer(vx_image image, void *virtAddr[], vx_uint32 sizes[])
{
    vx_status status = VX_SUCCESS;

    void      *plane_addr[TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    vx_uint32  plane_sizes[TIOVX_MODULES_MAX_REF_HANDLES];

    vx_size img_size;
    vx_uint32 num_planes;

    status = vxQueryImage(image, VX_IMAGE_SIZE, &img_size, sizeof(img_size));

    if((vx_status)VX_SUCCESS == status)
    {
        void *pBase = tivxMemAlloc(img_size, TIVX_MEM_EXTERNAL);

        /* Export handles to get valid size information. */
        status = tivxReferenceExportHandle((vx_reference)image,
                                            plane_addr,
                                            plane_sizes,
                                            TIOVX_MODULES_MAX_REF_HANDLES,
                                            &num_planes);

        if((vx_status)VX_SUCCESS == status)
        {
            vx_int32 p;
            vx_int32 prev_size = 0;
            for(p = 0; p < num_planes; p++)
            {
                virtAddr[p] = (void *)((vx_uint8 *)pBase + prev_size);
                sizes[p] = plane_sizes[p];
                prev_size += plane_sizes[p];
            }
        }
    }

    return status;
}

vx_status delete_single_image_buffer(vx_image image, void *virtAddr[], vx_uint32 sizes[])
{
    vx_status status = VX_SUCCESS;

    vx_size img_size;
    vx_size num_planes;

    status = vxQueryImage(image, VX_IMAGE_SIZE, &img_size, sizeof(img_size));

    if((vx_status)VX_SUCCESS == status)
    {
        status = vxQueryImage(image, VX_IMAGE_PLANES, &num_planes, sizeof(num_planes));
    }

    if((vx_status)VX_SUCCESS == status)
    {
        /* Free only the first plane_addr as the remaining ones were
            derrived in allocate_single_image_buffer */
        tivxMemFree(virtAddr[0], img_size, TIVX_MEM_EXTERNAL);

        /* Mark the handle and sizes as NULL and zero respectively */
        vx_int32 p;
        for(p = 0; p < num_planes; p++)
        {
            virtAddr[p] = NULL;
            sizes[p] = 0;
        }
    }

    return status;
}

vx_status assign_single_image_buffer(vx_image image, void *virtAddr[], vx_uint32 sizes[], vx_uint32 num_planes)
{
    vx_status status = VX_SUCCESS;

    if((vx_status)VX_SUCCESS == status)
    {
        void * addr[4];
        vx_int32 bufsize[4];
        vx_int32 p;

        for(p = 0; p < num_planes; p++)
        {
            addr[p] = virtAddr[p];
            bufsize[p] = sizes[p];
        }

        status = tivxReferenceImportHandle((vx_reference)image,
                                        (const void **)addr,
                                        (const uint32_t *)bufsize,
                                        num_planes);
    }

    return status;
}

vx_status release_single_image_buffer(vx_image image, void *virtAddr[], vx_uint32 sizes[], vx_uint32 num_planes)
{
    vx_status status = VX_SUCCESS;

    if((vx_status)VX_SUCCESS == status)
    {
        void * addr[4];
        vx_int32 bufsize[4];
        vx_int32 p;

        for(p = 0; p < num_planes; p++)
        {
            addr[p] = NULL;
            bufsize[p] = sizes[p];
        }

        /* Assign NULL handles to the OpenVx objects as it will avoid
            doing a tivxMemFree twice, once now and once during release */
        status = tivxReferenceImportHandle((vx_reference)image,
                                            (const void **)addr,
                                            (const uint32_t *)bufsize,
                                            num_planes);
    }

    return status;
}

vx_status allocate_image_buffers(ImgObj *imgObj, void *virtAddr[][TIOVX_MODULES_MAX_REF_HANDLES], vx_uint32 sizes[][TIOVX_MODULES_MAX_REF_HANDLES])
{
    vx_status status = VX_SUCCESS;

    vx_size num_ch;
    vx_int32 bufq, ch, ctr;
    vx_size num_planes;

    APP_PRINTF("Allocating Buffers \n");

    for(bufq = 0; bufq < imgObj->bufq_depth; bufq++)
    {
        vxQueryObjectArray(imgObj->arr[bufq], VX_OBJECT_ARRAY_NUMITEMS, &num_ch, sizeof(vx_size));

        if((vx_status)VX_SUCCESS == status)
        {
            vx_int32   l;

            ctr = 0;
            for(ch = 0; ch < num_ch; ch++)
            {
                vx_image image = (vx_image)vxGetObjectArrayItem(imgObj->arr[bufq], ch);
                status = vxQueryImage(image, VX_IMAGE_PLANES, &num_planes, sizeof(num_planes));

                if((vx_status)VX_SUCCESS == status)
                {
                    status = allocate_single_image_buffer
                            (
                                image,
                                &virtAddr[bufq][ctr],
                                &sizes[bufq][ctr]
                            );
                }

                for(l = 0; l < num_planes; l++)
                {
                    APP_PRINTF("virtAddr[%d][%d] = 0x%016lx, size = %d\n", bufq, l, (unsigned long int)virtAddr[bufq][ctr + l], sizes[bufq][ctr + l]);
                }

                ctr += num_planes;
                vxReleaseImage(&image);

                if((vx_status)VX_SUCCESS != status)
                {
                    APP_PRINTF("Unable to allocate single image buffer!\n");
                    break;
                }
            }
        }
    }

    return status;
}

vx_status delete_image_buffers(ImgObj *imgObj, void *virtAddr[][TIOVX_MODULES_MAX_REF_HANDLES], vx_uint32 sizes[][TIOVX_MODULES_MAX_REF_HANDLES])
{
    vx_status status = VX_SUCCESS;

    vx_int32 bufq, ch, ctr;

    vx_size num_ch, num_planes;

    APP_PRINTF("Deleting Buffers \n");


    for(bufq = 0; bufq < imgObj->bufq_depth; bufq++)
    {
        vx_int32 l;
        vxQueryObjectArray(imgObj->arr[bufq], VX_OBJECT_ARRAY_NUMITEMS, &num_ch, sizeof(vx_size));

        ctr = 0;
        for(ch = 0; ch < num_ch; ch++)
        {
            vx_image image = (vx_image)vxGetObjectArrayItem(imgObj->arr[bufq], ch);
            status = vxQueryImage(image, VX_IMAGE_PLANES, &num_planes, sizeof(num_planes));

            for(l = 0; l < num_planes; l++)
            {
                APP_PRINTF("virtAddr[%d][%d] = 0x%016lx, size = %d\n", bufq, l, (unsigned long int)virtAddr[bufq][ctr + l], sizes[bufq][ctr + l]);
            }

            if((vx_status)VX_SUCCESS == status)
            {
                status = delete_single_image_buffer
                        (
                            image,
                            &virtAddr[bufq][ctr],
                            &sizes[bufq][ctr]
                        );

                if((vx_status)VX_SUCCESS != status)
                {
                    APP_PRINTF("Unable to delete single image buffer!\n");
                    break;
                }
            }

            ctr += num_planes;
            vxReleaseImage(&image);
        }
    }

    return status;
}

vx_status assign_image_buffers(ImgObj *imgObj, void *virtAddr[], vx_uint32 sizes[], vx_int32 bufq)
{
    vx_status status = VX_SUCCESS;

    vx_int32 ch, ctr, l;
    vx_size num_ch;

    vxQueryObjectArray(imgObj->arr[bufq], VX_OBJECT_ARRAY_NUMITEMS, &num_ch, sizeof(vx_size));

    APP_PRINTF("Assigning Buffers \n");

    ctr = 0;
    for(ch = 0; ch < num_ch; ch++)
    {
        vx_size num_planes;

        vx_image image = (vx_image)vxGetObjectArrayItem(imgObj->arr[bufq], ch);
        status = vxQueryImage(image, VX_IMAGE_PLANES, &num_planes, sizeof(num_planes));

        for(l = 0; l < num_planes; l++)
        {
            APP_PRINTF("virtAddr[%d][%d] = 0x%016lx, size = %d\n", bufq, l, (unsigned long int)virtAddr[ctr + l], sizes[ctr + l]);
        }

        if((vx_status)VX_SUCCESS == status)
        {
            status = assign_single_image_buffer(image, &virtAddr[ctr], &sizes[ctr], num_planes);
        }

        if((vx_status)VX_SUCCESS != status)
        {
            APP_PRINTF("Unable to assign single image buffer!\n");
            break;
        }

        ctr += num_planes;
        vxReleaseImage(&image);
    }

    return status;
}

vx_status release_image_buffers(ImgObj *imgObj, void *virtAddr[], vx_uint32 sizes[], vx_int32 bufq)
{
    vx_status status = VX_SUCCESS;

    vx_int32 ch, ctr, l;
    vx_size num_ch;
    vx_size num_planes;

    vxQueryObjectArray(imgObj->arr[bufq], VX_OBJECT_ARRAY_NUMITEMS, &num_ch, sizeof(vx_size));

    APP_PRINTF("Releasing Buffers \n");

    ctr = 0;
    for(ch = 0; ch < num_ch; ch++)
    {
        vx_image image = (vx_image)vxGetObjectArrayItem(imgObj->arr[bufq], ch);
        status = vxQueryImage(image, VX_IMAGE_PLANES, &num_planes, sizeof(num_planes));

        for(l = 0; l < num_planes; l++)
        {
            APP_PRINTF("virtAddr[%d][%d] = 0x%016lx, size = %d\n", bufq, l, (unsigned long int)virtAddr[ctr + l], sizes[ctr + l]);
        }

        if((vx_status)VX_SUCCESS == status)
        {
            status = release_single_image_buffer(image, &virtAddr[ctr], &sizes[ctr], num_planes);
        }

        if((vx_status)VX_SUCCESS != status)
        {
            APP_PRINTF("Unable to assign single image buffer!\n");
            break;
        }

        ctr += num_planes;
        vxReleaseImage(&image);
    }

    return status;
}

vx_status allocate_single_tensor_buffer(vx_tensor tensor, void *virtAddr[], vx_uint32 sizes[])
{
    vx_status status = VX_SUCCESS;

    void      *buf_addr[TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    vx_uint32  buf_sizes[TIOVX_MODULES_MAX_REF_HANDLES];

    vx_size tensor_sizes[APP_MAX_TENSOR_DIMS];
    vx_size num_dims;
    vx_enum tensor_type;

    vxQueryTensor(tensor, VX_TENSOR_NUMBER_OF_DIMS, &num_dims, sizeof(vx_size));
    vxQueryTensor(tensor, VX_TENSOR_DIMS, tensor_sizes, num_dims * sizeof(vx_size));
    vxQueryTensor(tensor, VX_TENSOR_DATA_TYPE, &tensor_type, sizeof(vx_enum));

    if((vx_status)VX_SUCCESS == status)
    {
        uint32_t num_bufs;
        vx_int32 dim;
        vx_uint32 total_size = get_tensor_bitdepth(tensor_type);
        for(dim = 0; dim < num_dims; dim++)
        {
            total_size *= tensor_sizes[dim];
        }
        void *pBase = tivxMemAlloc(total_size, TIVX_MEM_EXTERNAL);

        /* Export handles to get valid size information. */
        status = tivxReferenceExportHandle((vx_reference)tensor,
                                            buf_addr,
                                            buf_sizes,
                                            TIOVX_MODULES_MAX_REF_HANDLES,
                                            &num_bufs);

        if((vx_status)VX_SUCCESS == status)
        {
            virtAddr[0] = (void *)pBase;
            sizes[0] = buf_sizes[0];
        }
    }

    return status;
}

vx_status delete_single_tensor_buffer(vx_tensor tensor, void *virtAddr[], vx_uint32 sizes[])
{
    vx_status status = VX_SUCCESS;

    vx_size tensor_sizes[APP_MAX_TENSOR_DIMS];
    vx_size num_dims;
    vx_enum tensor_type;

    vxQueryTensor(tensor, VX_TENSOR_NUMBER_OF_DIMS, &num_dims, sizeof(vx_size));
    vxQueryTensor(tensor, VX_TENSOR_DIMS, tensor_sizes, num_dims * sizeof(vx_size));
    vxQueryTensor(tensor, VX_TENSOR_DATA_TYPE, &tensor_type, sizeof(vx_enum));

    if((vx_status)VX_SUCCESS == status)
    {
        vx_int32 dim;
        vx_uint32 total_size = get_tensor_bitdepth(tensor_type);
        for(dim = 0; dim < num_dims; dim++)
        {
            total_size *= tensor_sizes[dim];
        }
        tivxMemFree(virtAddr[0], total_size, TIVX_MEM_EXTERNAL);

        virtAddr[0] = NULL;
        sizes[0] = 0;
    }

    return status;
}

vx_status assign_single_tensor_buffer(vx_tensor tensor, void *virtAddr[], vx_uint32 sizes[], vx_uint32 num_bufs)
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

        status = tivxReferenceImportHandle((vx_reference)tensor,
                                        (const void **)addr,
                                        (const uint32_t *)bufsize,
                                        num_bufs);
    }

    return status;
}

vx_status release_single_tensor_buffer(vx_tensor tensor, void *virtAddr[], vx_uint32 sizes[], vx_uint32 num_bufs)
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
        status = tivxReferenceImportHandle((vx_reference)tensor,
                                            (const void **)addr,
                                            (const uint32_t *)bufsize,
                                            num_bufs);
    }

    return status;
}

vx_status allocate_tensor_buffers(TensorObj *tensorObj, void *virtAddr[][TIOVX_MODULES_MAX_REF_HANDLES], vx_uint32 sizes[][TIOVX_MODULES_MAX_REF_HANDLES])
{
    vx_status status = VX_SUCCESS;

    vx_size num_ch;
    vx_int32 bufq, ch;

    APP_PRINTF("Allocating Tensor Buffers \n");

    for(bufq = 0; bufq < tensorObj->bufq_depth; bufq++)
    {
        vxQueryObjectArray(tensorObj->arr[bufq], VX_OBJECT_ARRAY_NUMITEMS, &num_ch, sizeof(vx_size));

        if((vx_status)VX_SUCCESS == status)
        {
            for(ch = 0; ch < num_ch; ch++)
            {
                vx_tensor tensor = (vx_tensor)vxGetObjectArrayItem(tensorObj->arr[bufq], ch);

                if((vx_status)VX_SUCCESS == status)
                {
                    status = allocate_single_tensor_buffer
                            (
                                tensor,
                                &virtAddr[bufq][ch],
                                &sizes[bufq][ch]
                            );
                }

                vxReleaseTensor(&tensor);

                APP_PRINTF("virtAddr[%d][%d] = 0x%016lx, size = %d\n", bufq, ch, (unsigned long int)virtAddr[bufq][ch], sizes[bufq][ch]);

                if((vx_status)VX_SUCCESS != status)
                {
                    APP_PRINTF("Unable to allocate single tensor buffer!\n");
                    break;
                }
            }
        }
    }

    return status;
}

vx_status delete_tensor_buffers(TensorObj *tensorObj, void *virtAddr[][TIOVX_MODULES_MAX_REF_HANDLES], vx_uint32 sizes[][TIOVX_MODULES_MAX_REF_HANDLES])
{
    vx_status status = VX_SUCCESS;

    vx_int32 bufq, ch;
    vx_size num_ch;

    APP_PRINTF("Deleting Tensor Buffers \n");
    for(bufq = 0; bufq < tensorObj->bufq_depth; bufq++)
    {
        vxQueryObjectArray(tensorObj->arr[bufq], VX_OBJECT_ARRAY_NUMITEMS, &num_ch, sizeof(vx_size));

        for(ch = 0; ch < num_ch; ch++)
        {
            vx_tensor tensor = (vx_tensor)vxGetObjectArrayItem(tensorObj->arr[bufq], ch);

            APP_PRINTF("virtAddr[%d][%d] = 0x%016lx, size = %d\n", bufq, ch, (unsigned long int)virtAddr[bufq][ch], sizes[bufq][ch]);

            if((vx_status)VX_SUCCESS == status)
            {
                status = delete_single_tensor_buffer
                        (
                            tensor,
                            &virtAddr[bufq][ch],
                            &sizes[bufq][ch]
                        );

                if((vx_status)VX_SUCCESS != status)
                {
                    APP_PRINTF("Unable to delete single tensor buffer!\n");
                    break;
                }
            }

            vxReleaseTensor(&tensor);
        }
    }

    return status;
}

vx_status assign_tensor_buffers(TensorObj *tensorObj, void *virtAddr[], vx_uint32 sizes[], vx_int32 bufq)
{
    vx_status status = VX_SUCCESS;

    vx_int32 ch;
    vx_size num_ch;

    vxQueryObjectArray(tensorObj->arr[bufq], VX_OBJECT_ARRAY_NUMITEMS, &num_ch, sizeof(vx_size));

    APP_PRINTF("Assigning Tensor Buffers \n");

    for(ch = 0; ch < num_ch; ch++)
    {
        vx_tensor tensor = (vx_tensor)vxGetObjectArrayItem(tensorObj->arr[bufq], ch);

        APP_PRINTF("virtAddr[%d][%d] = 0x%016lx, size = %d\n", bufq, ch, (unsigned long int)virtAddr[ch], sizes[ch]);

        if((vx_status)VX_SUCCESS == status)
        {
            status = assign_single_tensor_buffer(tensor, &virtAddr[ch], &sizes[ch], 1);
        }

        if((vx_status)VX_SUCCESS != status)
        {
            APP_PRINTF("Unable to assign single tensor buffer!\n");
            break;
        }

        vxReleaseTensor(&tensor);
    }

    return status;
}

vx_status release_tensor_buffers(TensorObj *tensorObj, void *virtAddr[], vx_uint32 sizes[], vx_int32 bufq)
{
    vx_status status = VX_SUCCESS;

    vx_int32 ch, ctr;
    vx_size num_ch;

    vxQueryObjectArray(tensorObj->arr[bufq], VX_OBJECT_ARRAY_NUMITEMS, &num_ch, sizeof(vx_size));

    APP_PRINTF("Releasing Tensor Buffers \n");

    ctr = 0;
    for(ch = 0; ch < num_ch; ch++)
    {
        vx_tensor tensor = (vx_tensor)vxGetObjectArrayItem(tensorObj->arr[bufq], ch);

        APP_PRINTF("virtAddr[%d][%d] = 0x%016lx, size = %d\n", bufq, ch, (unsigned long int)virtAddr[ch], sizes[ch]);

        if((vx_status)VX_SUCCESS == status)
        {
            status = release_single_tensor_buffer(tensor, &virtAddr[ctr], &sizes[ctr], 1);
        }

        if((vx_status)VX_SUCCESS != status)
        {
            APP_PRINTF("Unable to assign single tensor buffer!\n");
            break;
        }

        vxReleaseTensor(&tensor);
    }

    return status;
}

static vx_uint32 get_tensor_bitdepth(vx_enum tensor_type)
{
    vx_uint32 tensor_bitdepth = 0;

    if(tensor_type == VX_TYPE_UINT8)
    {
        tensor_bitdepth = sizeof(vx_uint8);
    }
    else if(tensor_type == VX_TYPE_INT8)
    {
        tensor_bitdepth = sizeof(vx_int8);
    }
    else if(tensor_type == VX_TYPE_UINT16)
    {
        tensor_bitdepth = sizeof(vx_uint16);
    }
    else if(tensor_type == VX_TYPE_INT16)
    {
        tensor_bitdepth = sizeof(vx_int16);
    }
    else if(tensor_type == VX_TYPE_UINT32)
    {
        tensor_bitdepth = sizeof(vx_uint32);
    }
    else if(tensor_type == VX_TYPE_INT32)
    {
        tensor_bitdepth = sizeof(vx_int32);
    }
    else if(tensor_type == VX_TYPE_FLOAT32)
    {
        tensor_bitdepth = sizeof(vx_float32);
    }

    return (tensor_bitdepth);
}

static vx_uint32 get_bit_depth(vx_enum data_type)
{
    vx_uint32 size = 0;

    if((data_type == VX_TYPE_UINT8) || (data_type == VX_TYPE_INT8))
    {
        size = sizeof(vx_uint8);
    }
    else if((data_type == VX_TYPE_UINT16) || (data_type == VX_TYPE_INT16))
    {
        size = sizeof(vx_uint16);
    }
    else if((data_type == VX_TYPE_UINT32) || (data_type == VX_TYPE_INT32))
    {
        size = sizeof(vx_uint32);
    }
    else if(data_type == VX_TYPE_FLOAT32)
    {
        size = sizeof(vx_float32);
    }

    return size;
}

vx_status readTensor(char* file_name, vx_tensor tensor_o)
{
    vx_status status = VX_SUCCESS;

    vx_size num_dims;
    void *data_ptr;
    vx_map_id map_id;

    vx_size start[APP_MAX_TENSOR_DIMS];
    vx_size tensor_strides[APP_MAX_TENSOR_DIMS];
    vx_size tensor_sizes[APP_MAX_TENSOR_DIMS];
    vx_char new_name[APP_MAX_FILE_PATH];
    vx_enum data_type;

    vxQueryTensor(tensor_o, VX_TENSOR_NUMBER_OF_DIMS, &num_dims, sizeof(vx_size));
    if(num_dims != 3)
    {
        APP_ERROR("Number of dims are != 3 \n");
        status = VX_FAILURE;
    }

    vxQueryTensor(tensor_o, VX_TENSOR_DATA_TYPE, &data_type, sizeof(vx_enum));
    vx_uint32 bit_depth = get_bit_depth(data_type);
    if(bit_depth == 0)
    {
        APP_ERROR("Incorrect data_type/bit-depth!\n \n");
        status = VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        vxQueryTensor(tensor_o, VX_TENSOR_DIMS, tensor_sizes, num_dims * sizeof(vx_size));

        start[0] = start[1] = start[2] = 0;

        tensor_strides[0] = bit_depth;
        tensor_strides[1] = tensor_sizes[0] * tensor_strides[0];
        tensor_strides[2] = tensor_sizes[1] * tensor_strides[1];

        status = tivxMapTensorPatch(tensor_o, num_dims, start, tensor_sizes, &map_id, tensor_strides, &data_ptr, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

        snprintf(new_name, APP_MAX_FILE_PATH, "%s_%dx%d.bin", file_name, (uint32_t)tensor_sizes[0], (uint32_t)tensor_sizes[1]);

        FILE *fp = fopen(new_name, "rb");
        if(NULL == fp)
        {
            APP_ERROR("Unable to open file %s \n", new_name);
            status = VX_FAILURE;
        }

        if(VX_SUCCESS == status)
        {
            int32_t size = fread(data_ptr, 1, tensor_sizes[0] * tensor_sizes[1] * tensor_sizes[2] * bit_depth, fp);
            if (size != (tensor_sizes[0] * tensor_sizes[1] * tensor_sizes[2] * bit_depth))
            {
                APP_ERROR("fread() size %d not matching with expected size! %d \n", size, (int32_t)(tensor_sizes[0] * tensor_sizes[1] * tensor_sizes[2] * bit_depth));
                status = VX_FAILURE;
            }

            tivxUnmapTensorPatch(tensor_o, map_id);
        }

        if(fp)
        {
            fclose(fp);
        }
    }

    return(status);
}

vx_status writeTensor(char* file_name, vx_tensor tensor_o)
{
    vx_status status = VX_SUCCESS;

    vx_size num_dims;
    void *data_ptr;
    vx_map_id map_id;

    vx_size start[APP_MAX_TENSOR_DIMS];
    vx_size tensor_strides[APP_MAX_TENSOR_DIMS];
    vx_size tensor_sizes[APP_MAX_TENSOR_DIMS];
    vx_char new_name[APP_MAX_FILE_PATH];
    vx_enum data_type;

    vxQueryTensor(tensor_o, VX_TENSOR_NUMBER_OF_DIMS, &num_dims, sizeof(vx_size));
    if(num_dims != 3)
    {
        APP_ERROR("Number of dims are != 3 \n");
        status = VX_FAILURE;
    }

    vxQueryTensor(tensor_o, VX_TENSOR_DATA_TYPE, &data_type, sizeof(vx_enum));
    vx_uint32 bit_depth = get_bit_depth(data_type);
    if(bit_depth == 0)
    {
        APP_ERROR("Incorrect data_type/bit-depth!\n \n");
        status = VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        vxQueryTensor(tensor_o, VX_TENSOR_DIMS, tensor_sizes, num_dims * sizeof(vx_size));

        start[0] = start[1] = start[2] = 0;

        tensor_strides[0] = bit_depth;
        tensor_strides[1] = tensor_sizes[0] * tensor_strides[0];
        tensor_strides[2] = tensor_sizes[1] * tensor_strides[1];

        status = tivxMapTensorPatch(tensor_o, num_dims, start, tensor_sizes, &map_id, tensor_strides, &data_ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);

        snprintf(new_name, APP_MAX_FILE_PATH, "%s_%dx%d.bin", file_name, (uint32_t)tensor_sizes[0], (uint32_t)tensor_sizes[1]);

        FILE *fp = fopen(new_name, "wb");
        if(NULL == fp)
        {
            APP_ERROR("Unable to open file %s \n", new_name);
            status = VX_FAILURE;
        }

        if(VX_SUCCESS == status)
        {
            int32_t size = fwrite(data_ptr, 1, tensor_sizes[0] * tensor_sizes[1] * tensor_sizes[2] * bit_depth, fp);
            if (size != (tensor_sizes[0] * tensor_sizes[1] * tensor_sizes[2] * bit_depth))
            {
                APP_ERROR("fwrite() size %d not matching with expected size! %d \n", size, (int32_t)(tensor_sizes[0] * tensor_sizes[1] * tensor_sizes[2] * bit_depth));
                status = VX_FAILURE;
            }

            tivxUnmapTensorPatch(tensor_o, map_id);
        }

        if(fp)
        {
            fclose(fp);
        }
    }

    return(status);
}

vx_status readRawImage(char* file_name, tivx_raw_image image)
{
    vx_status status;

    status = vxGetStatus((vx_reference)image);

    if((vx_status)VX_SUCCESS == status)
    {
        FILE * fp = fopen(file_name,"rb");
        vx_int32  j;

        if(fp == NULL)
        {
            APP_ERROR("Unable to open file %s \n", file_name);
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
                    VX_WRITE_ONLY,
                    VX_MEMORY_TYPE_HOST,
                    TIVX_RAW_IMAGE_PIXEL_BUFFER
                    );

                uint8_t *pIn = (uint8_t *)data_ptr;
                num_bytes = 0;
                if(line_interleaved == vx_true_e)
                {
                    for (j = 0; j < (image_addr.dim_y * num_exposures); j++)
                    {
                        num_bytes += fread(pIn, 1, image_addr.dim_x * bpp, fp);
                        pIn += image_addr.stride_y;
                    }
                }
                else
                {
                    for (j = 0; j < image_addr.dim_y; j++)
                    {
                        num_bytes += fread(pIn, 1, image_addr.dim_x * bpp, fp);
                        pIn += image_addr.stride_y;
                    }
                }

                plane_size = (image_addr.dim_y * image_addr.dim_x* bpp);

                if(num_bytes != plane_size)
                    APP_ERROR("Plane [%d] bytes read = %d, expected = %d\n", plane, num_bytes, plane_size);

                tivxUnmapRawImagePatch(image, map_id);
            }
        }

        fclose(fp);
    }

    return(status);
}

vx_status writeRawImage(char* file_name, tivx_raw_image image)
{
    vx_status status;

    status = vxGetStatus((vx_reference)image);

    if((vx_status)VX_SUCCESS == status)
    {
        FILE * fp = fopen(file_name,"wb");
        vx_int32  j;

        if(fp == NULL)
        {
            APP_ERROR("Unable to open file %s \n", file_name);
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
                    APP_ERROR("Plane [%d] bytes written = %d, expected = %d\n", plane, num_bytes, plane_size);

                tivxUnmapRawImagePatch(image, map_id);
            }
        }

        fclose(fp);
    }

    return(status);
}

vx_status readImage(char* file_name, vx_image img)
{
    vx_status status;

    status = vxGetStatus((vx_reference)img);

    if((vx_status)VX_SUCCESS == status)
    {
        FILE * fp = fopen(file_name,"rb");
        vx_int32  j;

        if(fp == NULL)
        {
            APP_ERROR("Unable to open file %s \n", file_name);
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
                                        VX_WRITE_ONLY,
                                        VX_MEMORY_TYPE_HOST,
                                        VX_NOGAP_X);

                APP_PRINTF("image_addr.dim_x = %d\n ", image_addr.dim_x);
                APP_PRINTF("image_addr.dim_y = %d\n ", image_addr.dim_y);
                APP_PRINTF("image_addr.step_x = %d\n ", image_addr.step_x);
                APP_PRINTF("image_addr.step_y = %d\n ", image_addr.step_y);
                APP_PRINTF("image_addr.stride_y = %d\n ", image_addr.stride_y);
                APP_PRINTF("image_addr.stride_x = %d\n ", image_addr.stride_x);
                APP_PRINTF("\n");

                num_bytes = 0;
                for (j = 0; j < (image_addr.dim_y/image_addr.step_y); j++)
                {
                    num_bytes += image_addr.stride_x * fread(data_ptr, image_addr.stride_x, (image_addr.dim_x/image_addr.step_x), fp);
                    data_ptr += image_addr.stride_y;
                }

                plane_size = (image_addr.dim_y/image_addr.step_y) * ((image_addr.dim_x * image_addr.stride_x)/image_addr.step_x);

                if(num_bytes != plane_size)
                    APP_ERROR("Plane [%d] bytes read = %d, expected = %d\n", plane, num_bytes, plane_size);

                vxUnmapImagePatch(img, map_id);
            }

        }

        fclose(fp);
    }

    return(status);
}

vx_status writeImage(char* file_name, vx_image img)
{
    vx_status status;

    status = vxGetStatus((vx_reference)img);

    if((vx_status)VX_SUCCESS == status)
    {
        FILE * fp = fopen(file_name,"wb");
        vx_int32  j;

        if(fp == NULL)
        {
            APP_ERROR("Unable to open file %s \n", file_name);
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

                APP_PRINTF("image_addr.dim_x = %d\n ", image_addr.dim_x);
                APP_PRINTF("image_addr.dim_y = %d\n ", image_addr.dim_y);
                APP_PRINTF("image_addr.step_x = %d\n ", image_addr.step_x);
                APP_PRINTF("image_addr.step_y = %d\n ", image_addr.step_y);
                APP_PRINTF("image_addr.stride_y = %d\n ", image_addr.stride_y);
                APP_PRINTF("image_addr.stride_x = %d\n ", image_addr.stride_x);
                APP_PRINTF("\n");

                num_bytes = 0;
                for (j = 0; j < (image_addr.dim_y/image_addr.step_y); j++)
                {
                    num_bytes += image_addr.stride_x * fwrite(data_ptr, image_addr.stride_x, (image_addr.dim_x/image_addr.step_x), fp);
                    data_ptr += image_addr.stride_y;
                }

                plane_size = (image_addr.dim_y/image_addr.step_y) * ((image_addr.dim_x * image_addr.stride_x)/image_addr.step_x);

                if(num_bytes != plane_size)
                    APP_ERROR("Plane [%d] bytes written = %d, expected = %d\n", plane, num_bytes, plane_size);

                vxUnmapImagePatch(img, map_id);
            }

        }

        fclose(fp);
    }

    return(status);
}

static inline void assign_class_ids(void *data_ptr, int32_t offset, vx_enum data_type, int32_t class_id)
{
    if(data_type == VX_TYPE_INT8)
    {
        *((int8_t *)data_ptr + offset) = class_id;
    }
    if(data_type == VX_TYPE_UINT8)
    {
        *((uint8_t *)data_ptr + offset) = class_id;
    }
    if(data_type == VX_TYPE_INT16)
    {
        *((int16_t *)data_ptr + offset) = class_id;
    }
    if(data_type == VX_TYPE_UINT16)
    {
        *((uint16_t *)data_ptr + offset) = class_id;
    }
    if(data_type == VX_TYPE_INT32)
    {
        *((int32_t *)data_ptr + offset) = class_id;
    }
    if(data_type == VX_TYPE_UINT32)
    {
        *((uint32_t *)data_ptr + offset) = class_id;
    }
    if(data_type == VX_TYPE_FLOAT32)
    {
        *((float *)data_ptr + offset) = class_id;
    }
}

vx_status create_tensor_mask(vx_tensor tensor_o, vx_int32 num_classes)
{
    vx_status status = VX_SUCCESS;

    vx_size num_dims;
    void *data_ptr;
    vx_map_id map_id;

    vx_size start[APP_MAX_TENSOR_DIMS];
    vx_size tensor_strides[APP_MAX_TENSOR_DIMS];
    vx_size tensor_sizes[APP_MAX_TENSOR_DIMS];
    vx_char new_name[APP_MAX_FILE_PATH];
    vx_enum data_type;

    vxQueryTensor(tensor_o, VX_TENSOR_NUMBER_OF_DIMS, &num_dims, sizeof(vx_size));
    if(num_dims != 3)
    {
        APP_ERROR("Number of dims are != 3 \n");
        status = VX_FAILURE;
    }

    vxQueryTensor(tensor_o, VX_TENSOR_DATA_TYPE, &data_type, sizeof(vx_enum));
    vx_uint32 bit_depth = get_bit_depth(data_type);
    if(bit_depth == 0)
    {
        APP_ERROR("Incorrect data_type/bit-depth!\n \n");
        status = VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        int32_t w, h;

        vxQueryTensor(tensor_o, VX_TENSOR_DIMS, tensor_sizes, num_dims * sizeof(vx_size));

        start[0] = start[1] = start[2] = 0;

        tensor_strides[0] = bit_depth;
        tensor_strides[1] = tensor_sizes[0] * tensor_strides[0];
        tensor_strides[2] = tensor_sizes[1] * tensor_strides[1];

        status = tivxMapTensorPatch(tensor_o, num_dims, start, tensor_sizes, &map_id, tensor_strides, &data_ptr, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

        for(h = 0; h < tensor_sizes[1]; h++)
        {
            for(w = 0; w < tensor_sizes[0]; w++)
            {
                int32_t offset = (h * tensor_sizes[0] + w);
                assign_class_ids(data_ptr, offset, data_type, ((h>>4) % num_classes));
            }
        }

        tivxUnmapTensorPatch(tensor_o, map_id);

    }

    return status;
}

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

vx_status allocate_pyramid_buffers(PyramidObj *pyramidObj, void *virtAddr[][TIOVX_MODULES_MAX_REF_HANDLES], vx_uint32 sizes[][TIOVX_MODULES_MAX_REF_HANDLES])
{
    vx_status status = VX_SUCCESS;

    vx_size num_ch;
    vx_int32 bufq, ch, ctr;
    vx_uint32 num_planes;

    APP_PRINTF("Allocating Buffers \n");

    for(bufq = 0; bufq < pyramidObj->bufq_depth; bufq++)
    {
        vxQueryObjectArray(pyramidObj->arr[bufq], VX_OBJECT_ARRAY_NUMITEMS, &num_ch, sizeof(vx_size));

        if((vx_status)VX_SUCCESS == status)
        {
            vx_int32   l;

            ctr = 0;
            for(ch = 0; ch < num_ch; ch++)
            {
                vx_pyramid pyramid = (vx_pyramid)vxGetObjectArrayItem(pyramidObj->arr[bufq], ch);

                status = allocate_single_pyramid_buffer
                        (
                            pyramid,
                            &virtAddr[bufq][ctr],
                            &sizes[bufq][ctr],
                            &num_planes
                        );

                for(l = 0; l < num_planes; l++)
                {
                    APP_PRINTF("virtAddr[%d][%d] = 0x%016lx, size = %d\n", bufq, l, (unsigned long int)virtAddr[bufq][ctr + l], sizes[bufq][ctr + l]);
                }

                ctr += num_planes;
                vxReleasePyramid(&pyramid);

                if((vx_status)VX_SUCCESS != status)
                {
                    APP_PRINTF("Unable to allocate single pyramid buffer!\n");
                    break;
                }
            }
        }
    }

    return status;
}

vx_status allocate_single_pyramid_buffer(vx_pyramid pyramid, void *virtAddr[], vx_uint32 sizes[], vx_uint32 *num_planes)
{
    vx_status status = VX_SUCCESS;

    void      *plane_addr[TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    vx_uint32  plane_sizes[TIOVX_MODULES_MAX_REF_HANDLES];

    vx_size   num_levels;
    vx_size   pyramid_size;
    vx_image  image;
    vx_size   img_size;

    status = vxQueryPyramid(pyramid, VX_PYRAMID_LEVELS, &num_levels, sizeof(num_levels));

    if((vx_status)VX_SUCCESS == status)
    {
        vx_int32 l;
        pyramid_size = 0;
        for(l = 0; l < num_levels; l++)
        {
            image = vxGetPyramidLevel(pyramid, l);
            status = vxQueryImage(image, VX_IMAGE_SIZE, &img_size, sizeof(img_size));
            pyramid_size += img_size;
            vxReleaseImage(&image);
        }
        void *pBase = tivxMemAlloc(pyramid_size, TIVX_MEM_EXTERNAL);

        /* Export handles to get valid size information. */
        status = tivxReferenceExportHandle((vx_reference)pyramid,
                                            plane_addr,
                                            plane_sizes,
                                            TIOVX_MODULES_MAX_REF_HANDLES,
                                            num_planes);

        if((vx_status)VX_SUCCESS == status)
        {
            vx_int32 p;
            vx_int32 prev_size = 0;
            for(p = 0; p < *num_planes; p++)
            {
                virtAddr[p] = (void *)((vx_uint8 *)pBase + prev_size);
                sizes[p] = plane_sizes[p];
                prev_size += plane_sizes[p];
            }
        }
    }

    return status;
}

vx_status assign_pyramid_buffers(PyramidObj *pyramidObj, void *virtAddr[], vx_uint32 sizes[], vx_int32 bufq)
{
    vx_status status = VX_SUCCESS;

    vx_int32 ch, ctr, l;
    vx_size num_ch;

    vxQueryObjectArray(pyramidObj->arr[bufq], VX_OBJECT_ARRAY_NUMITEMS, &num_ch, sizeof(vx_size));

    APP_PRINTF("Assigning Buffers \n");

    ctr = 0;
    for(ch = 0; ch < num_ch; ch++)
    {
        vx_uint32 num_planes;

        vx_pyramid pyramid = (vx_pyramid)vxGetObjectArrayItem(pyramidObj->arr[bufq], ch);

        status = assign_single_pyramid_buffer
                (
                    pyramid,
                    &virtAddr[ctr],
                    &sizes[ctr],
                    &num_planes
                );

        if((vx_status)VX_SUCCESS != status)
        {
            APP_PRINTF("Unable to assign single pyramid buffer!\n");
            break;
        }

        for(l = 0; l < num_planes; l++)
        {
            APP_PRINTF("virtAddr[%d][%d] = 0x%016lx, size = %d\n", bufq, l, (unsigned long int)virtAddr[ctr + l], sizes[ctr + l]);
        }

        ctr += num_planes;
        vxReleasePyramid(&pyramid);
    }

    return status;
}

vx_status assign_single_pyramid_buffer(vx_pyramid pyramid, void *virtAddr[], vx_uint32 sizes[], vx_uint32 *num_planes)
{
    vx_status status = VX_SUCCESS;
    void      *plane_addr[TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    vx_uint32  plane_sizes[TIOVX_MODULES_MAX_REF_HANDLES];

    if((vx_status)VX_SUCCESS == status)
    {
        /* Export handles to get valid size information. */
        status = tivxReferenceExportHandle((vx_reference)pyramid,
                                            plane_addr,
                                            plane_sizes,
                                            TIOVX_MODULES_MAX_REF_HANDLES,
                                            num_planes);

        status = tivxReferenceImportHandle((vx_reference)pyramid,
                                        (const void **)virtAddr,
                                        (const uint32_t *)sizes,
                                        *num_planes);
    }

    return status;
}

vx_status release_pyramid_buffers(PyramidObj *pyramidObj, void *virtAddr[], vx_uint32 sizes[], vx_int32 bufq)
{
    vx_status status = VX_SUCCESS;

    vx_int32 ch, ctr, l;
    vx_size num_ch;
    vx_uint32 num_planes;

    vxQueryObjectArray(pyramidObj->arr[bufq], VX_OBJECT_ARRAY_NUMITEMS, &num_ch, sizeof(vx_size));

    APP_PRINTF("Releasing Buffers \n");

    ctr = 0;
    for(ch = 0; ch < num_ch; ch++)
    {
        vx_pyramid pyramid = (vx_pyramid)vxGetObjectArrayItem(pyramidObj->arr[bufq], ch);

        status = release_single_pyramid_buffer
                (
                    pyramid,
                    &virtAddr[ctr],
                    &sizes[ctr],
                    &num_planes
                );

        if((vx_status)VX_SUCCESS != status)
        {
            APP_PRINTF("Unable to release single pyramid buffer!\n");
            break;
        }

        for(l = 0; l < num_planes; l++)
        {
            APP_PRINTF("virtAddr[%d][%d] = 0x%016lx, size = %d\n", bufq, l, (unsigned long int)virtAddr[ctr + l], sizes[ctr + l]);
        }

        ctr += num_planes;
        vxReleasePyramid(&pyramid);
    }

    return status;
}

vx_status release_single_pyramid_buffer(vx_pyramid pyramid, void *virtAddr[], vx_uint32 sizes[], vx_uint32 *num_planes)
{
    vx_status status = VX_SUCCESS;

    if((vx_status)VX_SUCCESS == status)
    {
        vx_int32 p;
        void      *plane_addr[TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
        vx_uint32  plane_sizes[TIOVX_MODULES_MAX_REF_HANDLES];

        /* Export handles to get valid size information. */
        status = tivxReferenceExportHandle((vx_reference)pyramid,
                                            plane_addr,
                                            plane_sizes,
                                            TIOVX_MODULES_MAX_REF_HANDLES,
                                            num_planes);
        for(p = 0; p < *num_planes; p++)
        {
            plane_addr[p] = NULL;
            plane_sizes[p] = sizes[p];
        }

        /* Assign NULL handles to the OpenVx objects as it will avoid
            doing a tivxMemFree twice, once now and once during release */
        status = tivxReferenceImportHandle((vx_reference)pyramid,
                                            (const void **)plane_addr,
                                            (const uint32_t *)plane_sizes,
                                            *num_planes);
    }

    return status;
}

vx_status delete_pyramid_buffers(PyramidObj *pyramidObj, void *virtAddr[][TIOVX_MODULES_MAX_REF_HANDLES], vx_uint32 sizes[][TIOVX_MODULES_MAX_REF_HANDLES])
{
    vx_status status = VX_SUCCESS;
    vx_int32 bufq, ch, ctr;
    vx_size num_ch;
    vx_uint32 num_planes;

    APP_PRINTF("Deleting Buffers \n");

    for(bufq = 0; bufq < pyramidObj->bufq_depth; bufq++)
    {
        vx_int32 l;
        vxQueryObjectArray(pyramidObj->arr[bufq], VX_OBJECT_ARRAY_NUMITEMS, &num_ch, sizeof(vx_size));

        ctr = 0;
        for(ch = 0; ch < num_ch; ch++)
        {
            vx_pyramid pyramid = (vx_pyramid)vxGetObjectArrayItem(pyramidObj->arr[bufq], ch);

            status = delete_single_pyramid_buffer
                    (
                        pyramid,
                        &virtAddr[bufq][ctr],
                        &sizes[bufq][ctr],
                        &num_planes
                    );

            if((vx_status)VX_SUCCESS != status)
            {
                APP_PRINTF("Unable to delete single pyramid buffer!\n");
                break;
            }

            for(l = 0; l < num_planes; l++)
            {
                APP_PRINTF("virtAddr[%d][%d] = 0x%016lx, size = %d\n", bufq, l, (unsigned long int)virtAddr[ctr + l], sizes[ctr + l]);
            }

            ctr += num_planes;
            vxReleasePyramid(&pyramid);
        }
    }

    return status;
}

vx_status delete_single_pyramid_buffer(vx_pyramid pyramid, void *virtAddr[], vx_uint32 sizes[], vx_uint32 *num_planes)
{
    vx_status status = VX_SUCCESS;

    if((vx_status)VX_SUCCESS == status)
    {
        vx_int32 p, l;
        void      *plane_addr[TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
        vx_uint32  plane_sizes[TIOVX_MODULES_MAX_REF_HANDLES];
        vx_size   num_levels;
        vx_size   pyramid_size;
        vx_image  image;
        vx_size   img_size;

        status = vxQueryPyramid(pyramid, VX_PYRAMID_LEVELS, &num_levels, sizeof(num_levels));

        /* Export handles to get valid size information. */
        status = tivxReferenceExportHandle((vx_reference)pyramid,
                                            plane_addr,
                                            plane_sizes,
                                            TIOVX_MODULES_MAX_REF_HANDLES,
                                            num_planes);
        pyramid_size = 0;
        for(l = 0; l < num_levels; l++)
        {
            image = vxGetPyramidLevel(pyramid, l);
            status = vxQueryImage(image, VX_IMAGE_SIZE, &img_size, sizeof(img_size));
            pyramid_size += img_size;
            vxReleaseImage(&image);
        }

        if((vx_status)VX_SUCCESS == status)
        {
            /* Free only the first plane_addr as the remaining ones were
                derrived in allocate_single_pyramid_buffer */
            tivxMemFree(virtAddr[0], pyramid_size, TIVX_MEM_EXTERNAL);

            /* Mark the handle and sizes as NULL and zero respectively */
            vx_int32 p;
            for(p = 0; p < *num_planes; p++)
            {
                virtAddr[p] = NULL;
                sizes[p] = 0;
            }
        }
    }

    return status;
}
