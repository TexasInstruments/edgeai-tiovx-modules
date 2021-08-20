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

static vx_uint32 get_tensor_bitdepth(vx_enum tensor_type);

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
        printf("Number of dims are != 3 \n");
        status = VX_FAILURE;
    }

    vxQueryTensor(tensor_o, VX_TENSOR_DATA_TYPE, &data_type, sizeof(vx_enum));
    vx_uint32 bit_depth = get_bit_depth(data_type);
    if(bit_depth == 0)
    {
        printf("Incorrect data_type/bit-depth!\n \n");
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

        snprintf(new_name, APP_MAX_FILE_PATH, "%s_%dx%d.bgr", file_name, (uint32_t)tensor_sizes[0], (uint32_t)tensor_sizes[1]);

        FILE *fp = fopen(new_name, "wb");
        if(NULL == fp)
        {
            printf("Unable to open file %s \n", new_name);
            status = VX_FAILURE;
        }

        if(VX_SUCCESS == status)
        {
            fwrite(data_ptr, 1, tensor_sizes[0] * tensor_sizes[1] * tensor_sizes[2] * bit_depth, fp);

            tivxUnmapTensorPatch(tensor_o, map_id);
        }

        if(fp)
        {
            fclose(fp);
        }
    }

    return(status);
}