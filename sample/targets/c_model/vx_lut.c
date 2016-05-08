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
 * \brief The TableLookup Kernel.
 * \author Erik Rainey <erik.rainey@gmail.com>
 */
#include <VX/vx.h>
#include <VX/vx_helper.h>

#include <vx_internal.h>
#include <c_model.h>

#include <math.h>

static vx_param_description_t lut_kernel_params[] = {
    { VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED },
    { VX_INPUT, VX_TYPE_LUT, VX_PARAMETER_STATE_REQUIRED },
    { VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED },
};

static vx_status VX_CALLBACK vxTableLookupKernel(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    if (num == dimof(lut_kernel_params))
    {
        vx_status status = VX_SUCCESS;
        vx_image src_image =  (vx_image)parameters[0];
        vx_lut   lut       =    (vx_lut)parameters[1];
        vx_image dst_image =  (vx_image)parameters[2];

        vx_bool is_replicated = vx_false_e;

        status = vxQueryNode(node, VX_NODE_IS_REPLICATED, &is_replicated, sizeof(is_replicated));
        if (VX_SUCCESS != status)
            return status;

        if (vx_true_e == is_replicated)
        {
            vx_size i;
            vx_bool replicas[VX_INT_MAX_PARAMS] = { vx_false_e };

            status = vxQueryNode(node, VX_NODE_REPLICATE_FLAGS, replicas, sizeof(vx_bool)*num);
            if (VX_SUCCESS != status)
                return status;

            /* if node is replicated, src and dst params have to be replicated */
            if (vx_true_e == replicas[0] &&
                vx_true_e != replicas[1] &&
                vx_true_e == replicas[2])
            {
                /* all params have to be pyramid (supported now) or image arrays (not implemented yet) */
                if (src_image->base.scope->type == VX_TYPE_PYRAMID && dst_image->base.scope->type == VX_TYPE_PYRAMID)
                {
                    vx_size pyr0_levels = 0;
                    vx_size pyr1_levels = 0;

                    vx_pyramid pyr0 = (vx_pyramid)src_image->base.scope;
                    vx_pyramid pyr1 = (vx_pyramid)dst_image->base.scope;

                    status = vxQueryPyramid(pyr0, VX_PYRAMID_LEVELS, &pyr0_levels, sizeof(pyr0_levels));
                    if (VX_SUCCESS != status)
                        return status;

                    status = vxQueryPyramid(pyr1, VX_PYRAMID_LEVELS, &pyr1_levels, sizeof(pyr1_levels));
                    if (VX_SUCCESS != status)
                        return status;

                    if (pyr0_levels != pyr1_levels)
                        return VX_FAILURE;

                    for (i = 0; i < pyr1_levels; i++)
                    {
                        vx_image src = vxGetPyramidLevel(pyr0, i);
                        vx_image dst = vxGetPyramidLevel(pyr1, i);

                        status = vxTableLookup(src, lut, dst);

                        status |= vxReleaseImage(&src);
                        status |= vxReleaseImage(&dst);
                    }
                }
            }
        }
        else
            status = vxTableLookup(src_image, lut, dst_image);

        return status;
    }

    return VX_ERROR_INVALID_PARAMETERS;
}

static vx_status VX_CALLBACK vxTableLookupInputValidator(vx_node node, vx_uint32 index)
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
            if (format == VX_DF_IMAGE_U8 || format == VX_DF_IMAGE_S16)
            {
                status = VX_SUCCESS;
            }
            vxReleaseImage(&input);
        }
        vxReleaseParameter(&param);
    }
    else if (index == 1)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        vx_lut lut = 0;
        vxQueryParameter(param, VX_PARAMETER_REF, &lut, sizeof(lut));
        if (lut)
        {
            vx_enum type = 0;
            vxQueryLUT(lut, VX_LUT_TYPE, &type, sizeof(type));
            if (type == VX_TYPE_UINT8 || type == VX_TYPE_INT16)
            {
                status = VX_SUCCESS;
            }
            vxReleaseLUT(&lut);
        }
        vxReleaseParameter(&param);
    }
    return status;
}

static vx_status VX_CALLBACK vxTableLookupOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t *ptr)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 2)
    {
        vx_parameter src_param = vxGetParameterByIndex(node, 0);
        if (vxGetStatus((vx_reference)src_param) == VX_SUCCESS)
        {
            vx_image src = 0;
            vxQueryParameter(src_param, VX_PARAMETER_REF, &src, sizeof(src));
            if (src)
            {
                vx_df_image format = 0;
                vx_uint32 width = 0, height = 0;

                vxQueryImage(src, VX_IMAGE_FORMAT, &format, sizeof(format));
                vxQueryImage(src, VX_IMAGE_WIDTH, &width, sizeof(height));
                vxQueryImage(src, VX_IMAGE_HEIGHT, &height, sizeof(height));
                /* output is equal type and size */
                ptr->type = VX_TYPE_IMAGE;
                ptr->dim.image.format = format;
                ptr->dim.image.width = width;
                ptr->dim.image.height = height;
                status = VX_SUCCESS;
                vxReleaseImage(&src);
            }
            vxReleaseParameter(&src_param);
        }
    }
    return status;
}

vx_kernel_description_t lut_kernel = {
    VX_KERNEL_TABLE_LOOKUP,
    "org.khronos.openvx.table_lookup",
    vxTableLookupKernel,
    lut_kernel_params, dimof(lut_kernel_params),
    NULL,
    vxTableLookupInputValidator,
    vxTableLookupOutputValidator,
    NULL,
    NULL,
};


