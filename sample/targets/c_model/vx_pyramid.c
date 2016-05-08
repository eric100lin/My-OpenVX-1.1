/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
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

/*!
 * \file
 * \brief The Image Pyramid Kernel
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <VX/vx.h>
#include <VX/vx_lib_debug.h>
#include <VX/vx_lib_extras.h>
#include <VX/vx_helper.h>

#include <vx_internal.h>
#include <math.h>


static const vx_uint32 gaussian5x5scale = 256;
static const vx_int16 gaussian5x5[5][5] =
{
    {1,  4,  6,  4, 1},
    {4, 16, 24, 16, 4},
    {6, 24, 36, 24, 6},
    {4, 16, 24, 16, 4},
    {1,  4,  6,  4, 1}
};

static vx_param_description_t gaussian_pyramid_kernel_params[] =
{
    {VX_INPUT,  VX_TYPE_IMAGE,   VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_PYRAMID, VX_PARAMETER_STATE_REQUIRED},
};

/*! \note Look at \ref vxGaussianPyramidNode to see how this pyramid construction works */


static vx_convolution vxCreateGaussian5x5Convolution(vx_context context)
{
    vx_convolution conv = vxCreateConvolution(context, 5, 5);
    vx_status status = vxCopyConvolutionCoefficients(conv, (vx_int16 *)gaussian5x5, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
    if (status != VX_SUCCESS)
    {
        vxReleaseConvolution(&conv);
        return NULL;
    }

    status = vxSetConvolutionAttribute(conv, VX_CONVOLUTION_SCALE, (void *)&gaussian5x5scale, sizeof(vx_uint32));
    if (status != VX_SUCCESS)
    {
        vxReleaseConvolution(&conv);
        return NULL;
    }
    return conv;
}


static vx_status VX_CALLBACK vxGaussianPyramidKernel(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_FAILURE;

    if (num == dimof(gaussian_pyramid_kernel_params))
    {
        vx_graph subgraph = vxGetChildGraphOfNode(node);
        status = vxProcessGraph(subgraph);
    }

    return status;
}


static vx_status VX_CALLBACK vxGaussianPyramidInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 0)
    {
        vx_image input = 0;
        vx_parameter param = vxGetParameterByIndex(node, index);

        vxQueryParameter(param, VX_PARAMETER_REF, &input, sizeof(input));
        if (input)
        {
            vx_df_image format = 0;
            vxQueryImage(input, VX_IMAGE_FORMAT, &format, sizeof(format));
            if (format == VX_DF_IMAGE_U8)
            {
                status = VX_SUCCESS;
            }
            vxReleaseImage(&input);
        }
        vxReleaseParameter(&param);
    }
    return status;
}


static vx_status VX_CALLBACK vxGaussianPyramidOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t *ptr)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 1)
    {
        vx_image src = 0;
        vx_parameter src_param = vxGetParameterByIndex(node, 0);
        vx_parameter dst_param = vxGetParameterByIndex(node, index);

        vxQueryParameter(src_param, VX_PARAMETER_REF, &src, sizeof(src));
        if (src)
        {
            vx_pyramid dst = 0;
            vxQueryParameter(dst_param, VX_PARAMETER_REF, &dst, sizeof(dst));

            if (dst)
            {
                vx_uint32 width = 0, height = 0;
                vx_df_image format;
                vx_size num_levels;
                vx_float32 scale;

                vxQueryImage(src, VX_IMAGE_WIDTH, &width, sizeof(width));
                vxQueryImage(src, VX_IMAGE_HEIGHT, &height, sizeof(height));
                vxQueryImage(src, VX_IMAGE_FORMAT, &format, sizeof(format));
                vxQueryPyramid(dst, VX_PYRAMID_LEVELS, &num_levels, sizeof(num_levels));
                vxQueryPyramid(dst, VX_PYRAMID_SCALE, &scale, sizeof(scale));

                /* fill in the meta data with the attributes so that the checker will pass */
                ptr->type = VX_TYPE_PYRAMID;
                ptr->dim.pyramid.width = width;
                ptr->dim.pyramid.height = height;
                ptr->dim.pyramid.format = format;
                ptr->dim.pyramid.levels = num_levels;
                ptr->dim.pyramid.scale = scale;
                status = VX_SUCCESS;
                vxReleasePyramid(&dst);
            }
            vxReleaseImage(&src);
        }
        vxReleaseParameter(&dst_param);
        vxReleaseParameter(&src_param);
    }
    return status;
}


static vx_status VX_CALLBACK vxGaussianPyramidInitializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;

    if (num == dimof(gaussian_pyramid_kernel_params))
    {
        vx_context context = vxGetContext((vx_reference)node);

        vx_graph subgraph = node->child;
        if (subgraph)
        {
            /* deallocate subgraph resources */
            status = vxReleaseGraph(&subgraph);
            if (VX_SUCCESS != status)
                return status;

            status = vxSetChildGraphOfNode(node, 0);
            if (VX_SUCCESS != status)
                return status;
        }

        status = vxLoadKernels(context, "openvx-debug");
        if (status != VX_SUCCESS)
            return status;

        /* allocate subgraph resources */
        subgraph = vxCreateGraph(context);

        status = vxGetStatus((vx_reference)subgraph);
        if (status == VX_SUCCESS)
        {
            vx_size lev;
            vx_size levels = 1;
            vx_border_t border;
            vx_image input = (vx_image)parameters[0];
            vx_pyramid gaussian = (vx_pyramid)parameters[1];
            vx_enum interp = VX_INTERPOLATION_NEAREST_NEIGHBOR;
            vx_convolution conv = vxCreateGaussian5x5Convolution(context);
            vx_image level0 = vxGetPyramidLevel(gaussian, 0);

            vx_node copy = vxCopyImageNode(subgraph, input, level0);

            status |= vxReleaseImage(&level0);

            /* We have a child-graph; we want to make sure the parent
            graph is recognized as a valid scope for sake of virtual
            image parameters. */
            subgraph->parentGraph = node->graph;

            status |= vxQueryNode(node, VX_NODE_BORDER, &border, sizeof(border));

            status |= vxQueryPyramid(gaussian, VX_PYRAMID_LEVELS, &levels, sizeof(levels));
            for (lev = 1; lev < levels; lev++)
            {
                /*! \internal Scaling is already precomputed for each level by the \ref vxCreatePyramid function */
                vx_image tmp0 = vxGetPyramidLevel(gaussian, (vx_uint32)lev - 1);
                vx_image tmp1 = vxGetPyramidLevel(gaussian, (vx_uint32)lev);
                vx_image virt = vxCreateVirtualImage(subgraph, 0, 0, VX_DF_IMAGE_U8);
                vx_node  gtmp = vxConvolveNode(subgraph, tmp0, conv, virt);
                vx_node  stmp = vxScaleImageNode(subgraph, virt, tmp1, interp);

                status |= vxSetNodeAttribute(gtmp, VX_NODE_BORDER, &border, sizeof(border));

                /* decrements the references */
                status |= vxReleaseImage(&tmp0);
                status |= vxReleaseImage(&tmp1);
                status |= vxReleaseImage(&virt);
                status |= vxReleaseNode(&gtmp);
                status |= vxReleaseNode(&stmp);
            }

            status |= vxAddParameterToGraphByIndex(subgraph, copy, 0); /* input image */
            status |= vxAddParameterToGraphByIndex(subgraph, node, 1); /* output pyramid - refer to self to quiet sub-graph validator */

            status |= vxReleaseNode(&copy);
            status |= vxReleaseConvolution(&conv);

            status |= vxVerifyGraph(subgraph);
            status |= vxSetChildGraphOfNode(node, subgraph);
        }
    }

    return status;
}


static vx_status VX_CALLBACK vxGaussianPyramidDeinitializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;

    if (num == dimof(gaussian_pyramid_kernel_params))
    {
        vx_graph subgraph = vxGetChildGraphOfNode(node);

        status = VX_SUCCESS;

        status |= vxReleaseGraph(&subgraph);

        /* set subgraph to "null" */
        status |= vxSetChildGraphOfNode(node, 0);
    }

    return status;
}


vx_kernel_description_t gaussian_pyramid_kernel =
{
    VX_KERNEL_GAUSSIAN_PYRAMID,
    "org.khronos.openvx.gaussian_pyramid",
    vxGaussianPyramidKernel,
    gaussian_pyramid_kernel_params, dimof(gaussian_pyramid_kernel_params),
    NULL,
    vxGaussianPyramidInputValidator,
    vxGaussianPyramidOutputValidator,
    vxGaussianPyramidInitializer,
    vxGaussianPyramidDeinitializer,
};


/* --------------------------------------------------------------------------*/
/* Laplacian Pyramid */

static vx_param_description_t laplacian_pyramid_kernel_params[] =
{
    { VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED },
    { VX_OUTPUT, VX_TYPE_PYRAMID, VX_PARAMETER_STATE_REQUIRED },
    { VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED },
};

static vx_status VX_CALLBACK vxLaplacianPyramidKernel(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_FAILURE;

    if (num == dimof(laplacian_pyramid_kernel_params))
    {
        vx_graph subgraph = vxGetChildGraphOfNode(node);
        status = vxProcessGraph(subgraph);
    }

    return status;
}


static vx_status VX_CALLBACK vxLaplacianPyramidInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_SUCCESS;
    if (index == 0)
    {
        vx_image input = 0;
        vx_parameter param = vxGetParameterByIndex(node, index);

        status |= vxQueryParameter(param, VX_PARAMETER_REF, &input, sizeof(input));
        if (status == VX_SUCCESS && input)
        {
            vx_df_image format = 0;
            status |= vxQueryImage(input, VX_IMAGE_FORMAT, &format, sizeof(format));
            if (status == VX_SUCCESS)
            {
                if (format != VX_DF_IMAGE_U8)
                    status = VX_ERROR_INVALID_PARAMETERS;
            }

            status |= vxReleaseImage(&input);
        }

        status |= vxReleaseParameter(&param);
    }

    return status;
}


static vx_status VX_CALLBACK vxLaplacianPyramidOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t* ptr)
{
    vx_status status = VX_SUCCESS;
    if (index == 1)
    {
        vx_image input = 0;
        vx_parameter src_param = vxGetParameterByIndex(node, 0);
        vx_parameter dst_param = vxGetParameterByIndex(node, index);

        status |= vxQueryParameter(src_param, VX_PARAMETER_REF, &input, sizeof(input));
        if (status == VX_SUCCESS && input)
        {
            vx_uint32 width = 0;
            vx_uint32 height = 0;
            vx_df_image format = 0;
            vx_pyramid laplacian = 0;

            status |= vxQueryImage(input, VX_IMAGE_WIDTH, &width, sizeof(width));
            status |= vxQueryImage(input, VX_IMAGE_HEIGHT, &height, sizeof(height));
            status |= vxQueryImage(input, VX_IMAGE_FORMAT, &format, sizeof(format));

            status |= vxQueryParameter(dst_param, VX_PARAMETER_REF, &laplacian, sizeof(laplacian));
            if (status == VX_SUCCESS && laplacian)
            {
                vx_uint32 pyr_width = 0;
                vx_uint32 pyr_height = 0;
                vx_df_image pyr_format = 0;
                vx_float32 pyr_scale = 0;
                vx_size pyr_levels = 0;

                status |= vxQueryPyramid(laplacian, VX_PYRAMID_WIDTH, &pyr_width, sizeof(pyr_width));
                status |= vxQueryPyramid(laplacian, VX_PYRAMID_HEIGHT, &pyr_height, sizeof(pyr_height));
                status |= vxQueryPyramid(laplacian, VX_PYRAMID_FORMAT, &pyr_format, sizeof(pyr_format));
                status |= vxQueryPyramid(laplacian, VX_PYRAMID_SCALE, &pyr_scale, sizeof(pyr_scale));
                status |= vxQueryPyramid(laplacian, VX_PYRAMID_LEVELS, &pyr_levels, sizeof(pyr_levels));

                if (status == VX_SUCCESS)
                {
                    if (pyr_width == width && pyr_height == height && pyr_format == VX_DF_IMAGE_S16 && pyr_scale == VX_SCALE_PYRAMID_HALF)
                    {
                        /* fill in the meta data with the attributes so that the checker will pass */
                        ptr->type = VX_TYPE_PYRAMID;
                        ptr->dim.pyramid.width = pyr_width;
                        ptr->dim.pyramid.height = pyr_height;
                        ptr->dim.pyramid.format = pyr_format;
                        ptr->dim.pyramid.scale = pyr_scale;
                        ptr->dim.pyramid.levels = pyr_levels;
                    }
                    else
                        status = VX_ERROR_INVALID_PARAMETERS;
                }

                status |= vxReleasePyramid(&laplacian);
            }

            status |= vxReleaseImage(&input);
        }

        status |= vxReleaseParameter(&dst_param);
        status |= vxReleaseParameter(&src_param);
    }

    if (index == 2)
    {
        vx_pyramid laplacian = 0;
        vx_parameter src_param = vxGetParameterByIndex(node, 1);
        vx_parameter dst_param = vxGetParameterByIndex(node, index);

        status |= vxQueryParameter(src_param, VX_PARAMETER_REF, &laplacian, sizeof(laplacian));
        if (status == VX_SUCCESS && laplacian)
        {
            vx_uint32 width = 0;
            vx_uint32 height = 0;
            vx_df_image format = 0;
            vx_size levels = 0;
            vx_image last_level = 0;
            vx_image output = 0;

            status |= vxQueryPyramid(laplacian, VX_PYRAMID_LEVELS, &levels, sizeof(levels));

            last_level = vxGetPyramidLevel(laplacian, levels - 1);

            status |= vxQueryImage(last_level, VX_IMAGE_WIDTH, &width, sizeof(width));
            status |= vxQueryImage(last_level, VX_IMAGE_HEIGHT, &height, sizeof(height));
            status |= vxQueryImage(last_level, VX_IMAGE_FORMAT, &format, sizeof(format));

            status |= vxQueryParameter(dst_param, VX_PARAMETER_REF, &output, sizeof(output));
            if (status == VX_SUCCESS && output)
            {
                vx_uint32 dst_width = 0;
                vx_uint32 dst_height = 0;
                vx_df_image dst_format = 0;

                status |= vxQueryImage(output, VX_IMAGE_WIDTH, &dst_width, sizeof(dst_width));
                status |= vxQueryImage(output, VX_IMAGE_HEIGHT, &dst_height, sizeof(dst_height));
                status |= vxQueryImage(output, VX_IMAGE_FORMAT, &dst_format, sizeof(dst_format));

                if (status == VX_SUCCESS)
                {
                    if (dst_width == width && dst_height == height && dst_format == VX_DF_IMAGE_S16)
                    {
                        /* fill in the meta data with the attributes so that the checker will pass */
                        ptr->type = VX_TYPE_IMAGE;
                        ptr->dim.image.width = dst_width;
                        ptr->dim.image.height = dst_height;
                        ptr->dim.image.format = dst_format;
                    }
                    else
                        status = VX_ERROR_INVALID_PARAMETERS;
                }

                status |= vxReleaseImage(&output);
            }

            status |= vxReleaseImage(&last_level);
            status |= vxReleasePyramid(&laplacian);
        }

        status |= vxReleaseParameter(&dst_param);
        status |= vxReleaseParameter(&src_param);
    }

    return status;
}


static vx_status VX_CALLBACK vxLaplacianPyramidInitializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;

    if (num == dimof(laplacian_pyramid_kernel_params))
    {
        vx_context context = vxGetContext((vx_reference)node);

        vx_graph subgraph = node->child;
        if (subgraph)
        {
            /* deallocate subgraph resources */
            status = vxReleaseGraph(&subgraph);
            if (VX_SUCCESS != status)
                return status;

            status = vxSetChildGraphOfNode(node, 0);
            if (VX_SUCCESS != status)
                return status;
        }

        status = vxLoadKernels(context, "openvx-debug");
        if (status != VX_SUCCESS)
            return status;

        /* allocate subgraph resources */
        subgraph = vxCreateGraph(context);

        status = vxGetStatus((vx_reference)subgraph);
        if (status == VX_SUCCESS)
        {
            vx_size lev;
            vx_size levels = 1;
            vx_uint32 width = 0;
            vx_uint32 height = 0;
            vx_uint32 level_width = 0;
            vx_uint32 level_height = 0;
            vx_df_image format = VX_DF_IMAGE_U8; /* only suported format for gaussian pyramids in OpenVX 1.1 */
            vx_enum interp = VX_INTERPOLATION_NEAREST_NEIGHBOR;
            vx_enum policy = VX_CONVERT_POLICY_WRAP;
            vx_border_t border;
            vx_convolution conv = 0;
            vx_image pyr_gauss_curr_level = 0;
            vx_image pyr_gauss_curr_level_filtered = 0;
            vx_image pyr_gauss_next_level = 0;
            vx_image pyr_laplacian_curr_level = 0;
            vx_node node_copy = 0;
            vx_node node_gauss = 0;
            vx_node node_scale = 0;
            vx_node node_sub = 0;
            vx_node node_cvt_depth = 0;
            vx_image   input = (vx_image)parameters[0];
            vx_pyramid laplacian = (vx_pyramid)parameters[1];
            vx_image   output = (vx_image)parameters[2];

            /* We have a child-graph; we want to make sure the parent
            graph is recognized as a valid scope for sake of virtual
            image parameters. */
            subgraph->parentGraph = node->graph;

            status |= vxQueryImage(input, VX_IMAGE_WIDTH, &width, sizeof(width));
            status |= vxQueryImage(input, VX_IMAGE_HEIGHT, &height, sizeof(height));

            status |= vxQueryPyramid(laplacian, VX_PYRAMID_LEVELS, &levels, sizeof(levels));

            status |= vxQueryNode(node, VX_NODE_BORDER, &border, sizeof(border));

            conv = vxCreateGaussian5x5Convolution(context);

            pyr_gauss_curr_level = vxCreateVirtualImage(subgraph, width, height, format);

            /* the first level of gaussian pyramid is just image itself */
            node_copy = vxCopyImageNode(subgraph, input, pyr_gauss_curr_level);

            level_width = width;
            level_height = height;

            for (lev = 0; lev < levels; lev++)
            {
                /* image of current level' dimension */
                pyr_gauss_curr_level_filtered = vxCreateVirtualImage(subgraph, level_width, level_height, format);

                /*! \internal compute the next level of gaussian pyramid */
                node_gauss = vxConvolveNode(subgraph, pyr_gauss_curr_level, conv, pyr_gauss_curr_level_filtered);

                status |= vxSetNodeAttribute(node_gauss, VX_NODE_BORDER, &border, sizeof(border));

                /* compute dimensions for the next level */
                level_width = (vx_uint32)ceilf(level_width * VX_SCALE_PYRAMID_HALF);
                level_height = (vx_uint32)ceilf(level_height * VX_SCALE_PYRAMID_HALF);

                /* image of the next level' dimension */
                pyr_gauss_next_level = vxCreateImage(context, level_width, level_height, format);

                node_scale = vxScaleImageNode(subgraph, pyr_gauss_curr_level_filtered, pyr_gauss_next_level, interp);

                /*! \internal compute curr level of laplacian pyramide */
                pyr_laplacian_curr_level = vxGetPyramidLevel(laplacian, lev);
                node_sub = vxSubtractNode(subgraph, pyr_gauss_curr_level, pyr_gauss_curr_level_filtered, policy, pyr_laplacian_curr_level);

                /* no need in the current level of gaussian pyramid anymore, so releasing it */
                vxReleaseImage(&pyr_gauss_curr_level);

                /*! \internal on the last iteration copy the image for reconstruction */
                if (lev == levels - 1)
                {
                    vx_int32 val = 0;
                    vx_scalar shift = vxCreateScalar(context, VX_TYPE_INT32, &val);
                    node_cvt_depth = vxConvertDepthNode(subgraph, pyr_gauss_curr_level_filtered, output, policy, shift);
                    status |= vxReleaseScalar(&shift);
                }
                else
                {
                    /* prepare to the next iteration */
                    /* make the next level of gaussian pyramid the current level */
                    pyr_gauss_curr_level = vxCreateVirtualImage(subgraph, level_width, level_height, format);
                    vx_node node_copy1 = vxCopyImageNode(subgraph, pyr_gauss_next_level, pyr_gauss_curr_level);
                    status |= vxReleaseNode(&node_copy1);
                }

                /* decrements the references */
                status |= vxReleaseImage(&pyr_gauss_curr_level_filtered);
                status |= vxReleaseImage(&pyr_gauss_next_level);
                status |= vxReleaseImage(&pyr_laplacian_curr_level);
                status |= vxReleaseNode(&node_gauss);
                status |= vxReleaseNode(&node_scale);
                status |= vxReleaseNode(&node_sub);
            }

            status |= vxAddParameterToGraphByIndex(subgraph, node_copy, 0);      /* input image */
            status |= vxAddParameterToGraphByIndex(subgraph, node, 1);           /* input pyramid - refer to self to quiet sub-graph validator */
            status |= vxAddParameterToGraphByIndex(subgraph, node_cvt_depth, 1); /* output image */

            status |= vxVerifyGraph(subgraph);

            status |= vxReleaseConvolution(&conv);
            status |= vxReleaseNode(&node_copy);
            status |= vxReleaseNode(&node_cvt_depth);

            status |= vxSetChildGraphOfNode(node, subgraph);
        }
    }

    return status;
}


static vx_status VX_CALLBACK vxLaplacianPyramidDeinitializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;

    if (num == dimof(laplacian_pyramid_kernel_params))
    {
        vx_graph subgraph = vxGetChildGraphOfNode(node);

        status = VX_SUCCESS;

        status |= vxReleaseGraph(&subgraph);

        /* set subgraph to "null" */
        status |= vxSetChildGraphOfNode(node, 0);
    }

    return status;
}


vx_kernel_description_t laplacian_pyramid_kernel =
{
    VX_KERNEL_LAPLACIAN_PYRAMID,
    "org.khronos.openvx.laplacian_pyramid",
    vxLaplacianPyramidKernel,
    laplacian_pyramid_kernel_params, dimof(laplacian_pyramid_kernel_params),
    NULL,
    vxLaplacianPyramidInputValidator,
    vxLaplacianPyramidOutputValidator,
    vxLaplacianPyramidInitializer,
    vxLaplacianPyramidDeinitializer,
};


/* Laplacian Reconstruct */

#define VX_SCALE_PYRAMID_DOUBLE (2.0f)

static vx_param_description_t laplacian_reconstruct_kernel_params[] =
{
    { VX_INPUT, VX_TYPE_PYRAMID, VX_PARAMETER_STATE_REQUIRED },
    { VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED },
    { VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED },
};

static vx_status VX_CALLBACK vxLaplacianReconstructKernel(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_FAILURE;

    if (num == dimof(laplacian_reconstruct_kernel_params))
    {
        vx_graph subgraph = vxGetChildGraphOfNode(node);
        status = vxProcessGraph(subgraph);
    }

    return status;
}


static vx_status VX_CALLBACK vxLaplacianReconstructInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_SUCCESS;
    if (index == 0)
    {
        vx_pyramid laplacian = 0;
        vx_parameter param = vxGetParameterByIndex(node, index);

        status |= vxQueryParameter(param, VX_PARAMETER_REF, &laplacian, sizeof(laplacian));
        if (status == VX_SUCCESS && laplacian)
        {
            vx_df_image pyr_format = 0;
            vx_float32  pyr_scale = 0;

            status |= vxQueryPyramid(laplacian, VX_PYRAMID_FORMAT, &pyr_format, sizeof(pyr_format));
            status |= vxQueryPyramid(laplacian, VX_PYRAMID_SCALE, &pyr_scale, sizeof(pyr_scale));

            if (status == VX_SUCCESS)
            {
                if ((pyr_format != VX_DF_IMAGE_S16) || (pyr_scale != VX_SCALE_PYRAMID_HALF))
                    status = VX_ERROR_INVALID_PARAMETERS;
            }

            status |= vxReleasePyramid(&laplacian);
        }

        status |= vxReleaseParameter(&param);
    }

    if (index == 1)
    {
        vx_pyramid laplacian = 0;
        vx_image input = 0;

        vx_parameter param0 = vxGetParameterByIndex(node, 0);
        vx_parameter param1 = vxGetParameterByIndex(node, index);

        status |= vxQueryParameter(param0, VX_PARAMETER_REF, &laplacian, sizeof(laplacian));
        status |= vxQueryParameter(param1, VX_PARAMETER_REF, &input, sizeof(input));
        if (status == VX_SUCCESS && laplacian && input)
        {
            vx_size     pyr_levels = 0;
            vx_uint32   pyr_last_level_width = 0;
            vx_uint32   pyr_last_level_height = 0;
            vx_image    pyr_last_level_img = 0;
            vx_uint32   lowest_res_width = 0;
            vx_uint32   lowest_res_height = 0;
            vx_df_image lowest_res_format = 0;

            status |= vxQueryPyramid(laplacian, VX_PYRAMID_LEVELS, &pyr_levels, sizeof(pyr_levels));

            pyr_last_level_img = vxGetPyramidLevel(laplacian, pyr_levels - 1);

            status |= vxQueryImage(pyr_last_level_img, VX_IMAGE_WIDTH, &pyr_last_level_width, sizeof(pyr_last_level_width));
            status |= vxQueryImage(pyr_last_level_img, VX_IMAGE_HEIGHT, &pyr_last_level_height, sizeof(pyr_last_level_height));

            status |= vxQueryImage(input, VX_IMAGE_WIDTH, &lowest_res_width, sizeof(lowest_res_width));
            status |= vxQueryImage(input, VX_IMAGE_HEIGHT, &lowest_res_height, sizeof(lowest_res_height));
            status |= vxQueryImage(input, VX_IMAGE_FORMAT, &lowest_res_format, sizeof(lowest_res_format));

            if (status == VX_SUCCESS)
            {
                if ((pyr_last_level_width != lowest_res_width) || (pyr_last_level_height != lowest_res_height) ||
                    (lowest_res_format != VX_DF_IMAGE_S16))
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
            }

            status |= vxReleasePyramid(&laplacian);
            status |= vxReleaseImage(&input);
            status |= vxReleaseImage(&pyr_last_level_img);
        }

        status |= vxReleaseParameter(&param0);
        status |= vxReleaseParameter(&param1);
    }

    return status;
}

static vx_status VX_CALLBACK vxLaplacianReconstructOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t* ptr)
{
    vx_status status = VX_SUCCESS;
    if (index == 2)
    {
        vx_pyramid laplacian = 0;
        vx_parameter src_param = vxGetParameterByIndex(node, 0);
        vx_parameter dst_param = vxGetParameterByIndex(node, index);

        status |= vxQueryParameter(src_param, VX_PARAMETER_REF, &laplacian, sizeof(laplacian));
        if (status == VX_SUCCESS && laplacian)
        {
            vx_uint32 pyr_width = 0;
            vx_uint32 pyr_height = 0;
            vx_image output = 0;

            status |= vxQueryPyramid(laplacian, VX_PYRAMID_WIDTH, &pyr_width, sizeof(pyr_width));
            status |= vxQueryPyramid(laplacian, VX_PYRAMID_HEIGHT, &pyr_height, sizeof(pyr_height));

            status |= vxQueryParameter(dst_param, VX_PARAMETER_REF, &output, sizeof(output));

            if (status == VX_SUCCESS && output)
            {
                vx_uint32 dst_width = 0;
                vx_uint32 dst_height = 0;
                vx_df_image dst_format = 0;

                status |= vxQueryImage(output, VX_IMAGE_WIDTH, &dst_width, sizeof(dst_width));
                status |= vxQueryImage(output, VX_IMAGE_HEIGHT, &dst_height, sizeof(dst_height));
                status |= vxQueryImage(output, VX_IMAGE_FORMAT, &dst_format, sizeof(dst_format));

                if (status == VX_SUCCESS)
                {
                    if (dst_width == pyr_width && dst_height == pyr_height && dst_format == VX_DF_IMAGE_U8)
                    {
                        /* fill in the meta data with the attributes so that the checker will pass */
                        ptr->type = VX_TYPE_IMAGE;
                        ptr->dim.image.width = dst_width;
                        ptr->dim.image.height = dst_height;
                        ptr->dim.image.format = dst_format;
                    }
                    else
                        status = VX_ERROR_INVALID_PARAMETERS;
                }

                status |= vxReleaseImage(&output);
            }

            status |= vxReleasePyramid(&laplacian);
        }

        status |= vxReleaseParameter(&dst_param);
        status |= vxReleaseParameter(&src_param);
    }

    return status;
}


static vx_status VX_CALLBACK vxLaplacianReconstructInitializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;

    if (num == dimof(laplacian_reconstruct_kernel_params))
    {
        vx_context context = vxGetContext((vx_reference)node);

        vx_graph subgraph = node->child;
        if (subgraph)
        {
            /* deallocate subgraph resources */
            status = vxReleaseGraph(&subgraph);
            if (VX_SUCCESS != status)
                return status;

            status = vxSetChildGraphOfNode(node, 0);
            if (VX_SUCCESS != status)
                return status;
        }

        status = vxLoadKernels(context, "openvx-debug");
        if (status != VX_SUCCESS)
            return status;

        /* allocate subgraph resources */
        subgraph = vxCreateGraph(context);

        status = vxGetStatus((vx_reference)subgraph);
        if (status == VX_SUCCESS)
        {
            vx_size lev;
            vx_size levels = 1;
            vx_uint32 width = 0;
            vx_uint32 height = 0;
            vx_uint32 level_width = 0;
            vx_uint32 level_height = 0;
            vx_df_image format = VX_DF_IMAGE_S16;
            vx_enum interp = VX_INTERPOLATION_NEAREST_NEIGHBOR;
            vx_enum policy = VX_CONVERT_POLICY_SATURATE;
            vx_border_t border;
            vx_image image_prev = 0;
            vx_image image_curr = 0;
            vx_image pyr_level = 0;
            vx_node node_copy = 0;
            vx_node node_scale = 0;
            vx_node node_add = 0;
            vx_node node_cvt_depth = 0;
            vx_pyramid laplacian = (vx_pyramid)parameters[0];
            vx_image   input = (vx_image)parameters[1];
            vx_image   output = (vx_image)parameters[2];

            /* We have a child-graph; we want to make sure the parent
            graph is recognized as a valid scope for sake of virtual
            image parameters. */
            subgraph->parentGraph = node->graph;

            status |= vxQueryImage(input, VX_IMAGE_WIDTH, &width, sizeof(width));
            status |= vxQueryImage(input, VX_IMAGE_HEIGHT, &height, sizeof(height));

            status |= vxQueryPyramid(laplacian, VX_PYRAMID_LEVELS, &levels, sizeof(levels));

            status |= vxQueryNode(node, VX_NODE_BORDER, &border, sizeof(border));

            image_prev = vxCreateVirtualImage(subgraph, width, height, format);

            node_copy = vxCopyImageNode(subgraph, input, image_prev);

            level_width = width;
            level_height = height;

            for (lev = 0; lev < levels; lev++)
            {
                image_curr = vxCreateVirtualImage(subgraph, level_width, level_height, format);

                pyr_level = vxGetPyramidLevel(laplacian, (levels - 1) - lev);
                node_add = vxAddNode(subgraph, pyr_level, image_prev, policy, image_curr);
                status |= vxReleaseNode(&node_add);

                /* no need in the current level of laplacian pyramid anymore, so releasing it */
                status |= vxReleaseImage(&pyr_level);
                /* no need in the image prev anymore */
                status |= vxReleaseImage(&image_prev);

                if ((levels - 1) - lev == 0)
                {
                    vx_int32 val = 0;
                    vx_scalar shift = vxCreateScalar(context, VX_TYPE_INT32, &val);
                    node_cvt_depth = vxConvertDepthNode(subgraph, image_curr, output, policy, shift);
                    status |= vxReleaseScalar(&shift);
                }
                else
                {
                    /* compute dimensions for the next level */
                    level_width = (vx_uint32)ceilf(level_width  * VX_SCALE_PYRAMID_DOUBLE);
                    level_height = (vx_uint32)ceilf(level_height * VX_SCALE_PYRAMID_DOUBLE);

                    /* create image of the prev level' dimension */
                    image_prev = vxCreateVirtualImage(subgraph, level_width, level_height, format);

                    /* upsample curr image to the prev level */
                    node_scale = vxScaleImageNode(subgraph, image_curr, image_prev, interp);
                    status |= vxReleaseNode(&node_scale);
                }

                /* no need in the curr image anymore */
                status |= vxReleaseImage(&image_curr);
            }

            status |= vxAddParameterToGraphByIndex(subgraph, node, 0);           /* input pyramid - refer to self to quiet sub-graph validator */
            status |= vxAddParameterToGraphByIndex(subgraph, node_copy, 0);      /* input image */
            status |= vxAddParameterToGraphByIndex(subgraph, node_cvt_depth, 1); /* output image */

            status |= vxVerifyGraph(subgraph);

            status |= vxReleaseNode(&node_copy);
            status |= vxReleaseNode(&node_cvt_depth);

            status |= vxSetChildGraphOfNode(node, subgraph);
        }
    }

    return status;
}


static vx_status VX_CALLBACK vxLaplacianReconstructDeinitializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;

    if (num == dimof(laplacian_reconstruct_kernel_params))
    {
        vx_graph subgraph = vxGetChildGraphOfNode(node);

        status = VX_SUCCESS;

        status |= vxReleaseGraph(&subgraph);

        /* set subgraph to "null" */
        status |= vxSetChildGraphOfNode(node, 0);
    }

    return status;
}


vx_kernel_description_t laplacian_reconstruct_kernel =
{
    VX_KERNEL_LAPLACIAN_RECONSTRUCT,
    "org.khronos.openvx.laplacian_reconstruct",
    vxLaplacianReconstructKernel,
    laplacian_reconstruct_kernel_params, dimof(laplacian_reconstruct_kernel_params),
    NULL,
    vxLaplacianReconstructInputValidator,
    vxLaplacianReconstructOutputValidator,
    vxLaplacianReconstructInitializer,
    vxLaplacianReconstructDeinitializer,
};
