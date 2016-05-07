/*
 * Copyright (c) 2013-2016 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */

#include <stdio.h>
#include <VX/vx.h>
#include <VX/vx_khr_tiling.h>
#include <VX/vx_lib_debug.h>
#include <VX/vx_helper.h>
#include "vx_tiling_ext.h"

/*! \file
 * \brief An example of how to call the tiling nodes.
 * \example vx_tiling_main.c
 */

vx_node vxTilingAddNode(vx_graph graph, vx_image in0, vx_image in1, vx_image out)
{
    vx_reference params[] = {
        (vx_reference)in0,
        (vx_reference)in1,
        (vx_reference)out,
    };
    return vxCreateNodeByStructure(graph,
                                    VX_KERNEL_ADD_TILING,
                                    params,
                                    dimof(params));
}

vx_node vxTilingAlphaNode(vx_graph graph, vx_image in, vx_scalar alpha, vx_image out)
{
    vx_reference params[] = {
        (vx_reference)in,
        (vx_reference)alpha,
        (vx_reference)out,
    };
    return vxCreateNodeByStructure(graph,
                                    VX_KERNEL_ALPHA_TILING,
                                    params,
                                    dimof(params));
}

vx_node vxTilingBoxNode(vx_graph graph, vx_image in, vx_image out, vx_uint32 width, vx_uint32 height)
{
    vx_reference params[] = {
        (vx_reference)in,
        (vx_reference)out,
    };
    vx_node node = vxCreateNodeByStructure(graph,
                                    VX_KERNEL_BOX_MxN_TILING,
                                    params,
                                    dimof(params));
    if (node && (width&1) && (height&1))
    {
        vx_neighborhood_size_t nbhd;
        vxQueryNode(node, VX_NODE_INPUT_NEIGHBORHOOD, &nbhd, sizeof(nbhd));
        nbhd.left = 0 - ((width - 1)/2);
        nbhd.right = ((width - 1)/2);
        nbhd.top = 0 - ((height - 1)/2);
        nbhd.bottom = ((height - 1)/2);
        vxSetNodeAttribute(node, VX_NODE_INPUT_NEIGHBORHOOD, &nbhd, sizeof(nbhd));
    }
    return node;
}

vx_node vxTilingGaussianNode(vx_graph graph, vx_image in, vx_image out)
{
    vx_reference params[] = {
        (vx_reference)in,
        (vx_reference)out,
    };
    return vxCreateNodeByStructure(graph,
                                    VX_KERNEL_GAUSSIAN_3x3_TILING,
                                    params,
                                    dimof(params));
}

int main(int argc, char *argv[])
{
    vx_status status = VX_SUCCESS;
    vx_context context = vxCreateContext();
    if (vxGetStatus((vx_reference)context) == VX_SUCCESS)
    {
        vx_rectangle_t rect = {1, 1, 513, 513}; // 512x512
        vx_uint32 i = 0;
        vx_image images[] = {
                vxCreateImage(context, 514, 514, VX_DF_IMAGE_U8), // 0:input
                vxCreateImageFromROI(images[0], &rect),       // 1:ROI input
                vxCreateImage(context, 512, 512, VX_DF_IMAGE_U8), // 2:box
                vxCreateImage(context, 512, 512, VX_DF_IMAGE_U8), // 3:gaussian
                vxCreateImage(context, 512, 512, VX_DF_IMAGE_U8), // 4:alpha
                vxCreateImage(context, 512, 512, VX_DF_IMAGE_S16),// 5:add
        };
        vx_float32 a = 0.5f;
        vx_scalar alpha = vxCreateScalar(context, VX_TYPE_FLOAT32, &a);
        status |= vxLoadKernels(context, "openvx-tiling");
        status |= vxLoadKernels(context, "openvx-debug");
        if (status == VX_SUCCESS)
        {
            vx_graph graph = vxCreateGraph(context);
            if (vxGetStatus((vx_reference)graph) == VX_SUCCESS)
            {
                vx_node nodes[] = {
                    vxFReadImageNode(graph, "lena_512x512.pgm", images[1]),
                    vxTilingBoxNode(graph, images[1], images[2], 5, 5),
                    vxFWriteImageNode(graph, images[2], "ot_box_lena_512x512.pgm"),
                    vxTilingGaussianNode(graph, images[1], images[3]),
                    vxFWriteImageNode(graph, images[3], "ot_gauss_lena_512x512.pgm"),
                    vxTilingAlphaNode(graph, images[1], alpha, images[4]),
                    vxFWriteImageNode(graph, images[4], "ot_alpha_lena_512x512.pgm"),
                    vxTilingAddNode(graph, images[1], images[4], images[5]),
                    vxFWriteImageNode(graph, images[5], "ot_add_lena_512x512.pgm"),
                };
                for (i = 0; i < dimof(nodes); i++)
                {
                    if (nodes[i] == 0)
                    {
                        printf("Failed to create node[%u]\n", i);
                        status = VX_ERROR_INVALID_NODE;
                        break;
                    }
                }
                if (status == VX_SUCCESS)
                {
                    status = vxVerifyGraph(graph);
                    if (status == VX_SUCCESS)
                    {
                        status = vxProcessGraph(graph);
                    }
                }
                for (i = 0; i < dimof(nodes); i++)
                {
                    vxReleaseNode(&nodes[i]);
                }
                vxReleaseGraph(&graph);
            }
        }
        status |= vxUnloadKernels(context, "openvx-debug");
        status |= vxUnloadKernels(context, "openvx-tiling");
        for (i = 0; i < dimof(images); i++)
        {
            vxReleaseImage(&images[i]);
        }
        vxReleaseContext(&context);
    }
    printf("%s::main() returns = %d\n", argv[0], status);
    return (int)status;
}
