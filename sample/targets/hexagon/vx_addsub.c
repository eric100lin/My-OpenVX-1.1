/*
 * Copyright (c) 2016 National Tsing Hua University.
 * \brief The OpenCL OpenVX Kernel Interfaces
 * \author Tzu-Hsiang Lin <thlin@pllab.cs.nthu.edu.tw>
 */

#include <VX/vx.h>
#include <VX/vx_helper.h>

#include <vx_internal.h>
#include <hexagon_model.h>

static vx_status VX_CALLBACK vxAddSubtractInputValidator(vx_node node, vx_uint32 index)
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
                status = VX_SUCCESS;
            vxReleaseImage(&input);
        }
        vxReleaseParameter(&param);
    }
    else if (index == 1)
    {
        vx_image images[2];
        vx_parameter param[2] = {
            vxGetParameterByIndex(node, 0),
            vxGetParameterByIndex(node, 1),
        };
        vxQueryParameter(param[0], VX_PARAMETER_REF, &images[0], sizeof(images[0]));
        vxQueryParameter(param[1], VX_PARAMETER_REF, &images[1], sizeof(images[1]));
        if (images[0] && images[1])
        {
            vx_uint32 width[2], height[2];
            vx_df_image format1;

            vxQueryImage(images[0], VX_IMAGE_WIDTH, &width[0], sizeof(width[0]));
            vxQueryImage(images[1], VX_IMAGE_WIDTH, &width[1], sizeof(width[1]));
            vxQueryImage(images[0], VX_IMAGE_HEIGHT, &height[0], sizeof(height[0]));
            vxQueryImage(images[1], VX_IMAGE_HEIGHT, &height[1], sizeof(height[1]));
            vxQueryImage(images[1], VX_IMAGE_FORMAT, &format1, sizeof(format1));
            if (width[0] == width[1] && height[0] == height[1] &&
                (format1 == VX_DF_IMAGE_U8 || format1 == VX_DF_IMAGE_S16))
                status = VX_SUCCESS;
            vxReleaseImage(&images[0]);
            vxReleaseImage(&images[1]);
        }
        vxReleaseParameter(&param[0]);
        vxReleaseParameter(&param[1]);
    }
    else if (index == 2)        /* overflow_policy: truncate or saturate. */
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (vxGetStatus((vx_reference)param) == VX_SUCCESS)
        {
            vx_scalar scalar = 0;
            vxQueryParameter(param, VX_PARAMETER_REF, &scalar, sizeof(scalar));
            if (scalar)
            {
                vx_enum stype = 0;
                vxQueryScalar(scalar, VX_SCALAR_TYPE, &stype, sizeof(stype));
                if (stype == VX_TYPE_ENUM)
                {
                    vx_enum overflow_policy = 0;
                    vxCopyScalar(scalar, &overflow_policy, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
                    if ((overflow_policy == VX_CONVERT_POLICY_WRAP) ||
                        (overflow_policy == VX_CONVERT_POLICY_SATURATE))
                    {
                        status = VX_SUCCESS;
                    }
                    else
                    {
                        status = VX_ERROR_INVALID_VALUE;
                    }
                }
                else
                {
                    status = VX_ERROR_INVALID_TYPE;
                }
                vxReleaseScalar(&scalar);
            }
            vxReleaseParameter(&param);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxAddSubtractOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t *ptr)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 3)
    {
        /*
         * We need to look at both input images, but only for the format:
         * if either is S16 or the output type is not U8, then it's S16.
         * The geometry of the output image is copied from the first parameter:
         * the input images are known to match from input parameters validation.
         */
        vx_parameter param[] = {
            vxGetParameterByIndex(node, 0),
            vxGetParameterByIndex(node, 1),
            vxGetParameterByIndex(node, index),
        };
        if ((vxGetStatus((vx_reference)param[0]) == VX_SUCCESS) &&
            (vxGetStatus((vx_reference)param[1]) == VX_SUCCESS) &&
            (vxGetStatus((vx_reference)param[2]) == VX_SUCCESS))
        {
            vx_image images[3];
            vxQueryParameter(param[0], VX_PARAMETER_REF, &images[0], sizeof(images[0]));
            vxQueryParameter(param[1], VX_PARAMETER_REF, &images[1], sizeof(images[1]));
            vxQueryParameter(param[2], VX_PARAMETER_REF, &images[2], sizeof(images[2]));
            if (images[0] && images[1] && images[2])
            {
                vx_uint32 width = 0, height = 0;
                vx_df_image informat[2] = {VX_DF_IMAGE_VIRT, VX_DF_IMAGE_VIRT};
                vx_df_image outformat = VX_DF_IMAGE_VIRT;

                /*
                 * When passing on the geometry to the output image, we only look at
                 * image 0, as both input images are verified to match, at input
                 * validation.
                 */
                vxQueryImage(images[0], VX_IMAGE_WIDTH, &width, sizeof(width));
                vxQueryImage(images[0], VX_IMAGE_HEIGHT, &height, sizeof(height));
                vxQueryImage(images[0], VX_IMAGE_FORMAT, &informat[0], sizeof(informat[0]));
                vxQueryImage(images[1], VX_IMAGE_FORMAT, &informat[1], sizeof(informat[1]));
                vxQueryImage(images[2], VX_IMAGE_FORMAT, &outformat, sizeof(outformat));

                if (informat[0] == VX_DF_IMAGE_U8 && informat[1] == VX_DF_IMAGE_U8 && outformat == VX_DF_IMAGE_U8)
                {
                    status = VX_SUCCESS;
                }
                else
                {
                    outformat = VX_DF_IMAGE_S16;
                    status = VX_SUCCESS;
                }
                ptr->type = VX_TYPE_IMAGE;
                ptr->dim.image.format = outformat;
                ptr->dim.image.width = width;
                ptr->dim.image.height = height;
                vxReleaseImage(&images[0]);
                vxReleaseImage(&images[1]);
                vxReleaseImage(&images[2]);
            }
            vxReleaseParameter(&param[0]);
            vxReleaseParameter(&param[1]);
            vxReleaseParameter(&param[2]);
        }
    }

    return status;
}

static vx_param_description_t add_subtract_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

static vx_status VX_CALLBACK vxAdditionKernel(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    if (num == dimof(add_subtract_kernel_params))
    {
        vx_status status = VX_SUCCESS;
        vx_image  in0          =  (vx_image)parameters[0];
        vx_image  in1          =  (vx_image)parameters[1];
        vx_scalar policy_param = (vx_scalar)parameters[2];
        vx_image  output       =  (vx_image)parameters[3];

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

            /* if node is replicated, in0, in1 and output params have to be replicated */
            if (vx_true_e == replicas[0] && vx_true_e == replicas[1] &&
                vx_true_e != replicas[2] &&
                vx_true_e == replicas[3])
            {
                /* all params have to be pyramid (supported now) or image arrays (not implemented yet) */
                if (in0->base.scope->type == VX_TYPE_PYRAMID && in1->base.scope->type == VX_TYPE_PYRAMID &&
                    output->base.scope->type == VX_TYPE_PYRAMID)
                {
                    vx_size pyr0_levels = 0;
                    vx_size pyr1_levels = 0;
                    vx_size pyr3_levels = 0;

                    vx_pyramid pyr0 = (vx_pyramid)in0->base.scope;
                    vx_pyramid pyr1 = (vx_pyramid)in1->base.scope;
                    vx_pyramid pyr3 = (vx_pyramid)output->base.scope;

                    status = vxQueryPyramid(pyr0, VX_PYRAMID_LEVELS, &pyr0_levels, sizeof(pyr0_levels));
                    if (VX_SUCCESS != status)
                        return status;

                    status = vxQueryPyramid(pyr1, VX_PYRAMID_LEVELS, &pyr1_levels, sizeof(pyr1_levels));
                    if (VX_SUCCESS != status)
                        return status;

                    status = vxQueryPyramid(pyr3, VX_PYRAMID_LEVELS, &pyr3_levels, sizeof(pyr3_levels));
                    if (VX_SUCCESS != status)
                        return status;

                    if (pyr0_levels != pyr1_levels || pyr0_levels != pyr3_levels)
                        return VX_FAILURE;

                    for (i = 0; i < pyr3_levels; i++)
                    {
                        vx_image src0 = vxGetPyramidLevel(pyr0, i);
                        vx_image src1 = vxGetPyramidLevel(pyr1, i);
                        vx_image dst  = vxGetPyramidLevel(pyr3, i);

                        status = vxAddition(node, src0, src1, policy_param, dst);

                        status |= vxReleaseImage(&src0);
                        status |= vxReleaseImage(&src1);
                        status |= vxReleaseImage(&dst);
                    }
                }
            }
        }
        else
            status = vxAddition(node, in0, in1, policy_param, output);

        return status;
    }

    return VX_ERROR_INVALID_PARAMETERS;
}

vx_kernel_description_t add_kernel = {
	VX_KERNEL_ADD,
	"org.khronos.openvx.add",
	vxAdditionKernel,
	add_subtract_kernel_params, dimof(add_subtract_kernel_params),
	NULL,
	vxAddSubtractInputValidator,
	vxAddSubtractOutputValidator,
	NULL,
    NULL,
};

static vx_status VX_CALLBACK vxSubtractionKernel(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    if (num == dimof(add_subtract_kernel_params))
    {
        vx_status status = VX_SUCCESS;
        vx_image  in0           =  (vx_image)parameters[0];
        vx_image  in1           =  (vx_image)parameters[1];
        vx_scalar policy_param  = (vx_scalar)parameters[2];
        vx_image  output        =  (vx_image)parameters[3];

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

            /* if node is replicated, in0, in1 and output params have to be replicated */
            if (vx_true_e == replicas[0] && vx_true_e == replicas[1] &&
                vx_true_e != replicas[2] &&
                vx_true_e == replicas[3])
            {
                /* all params have to be pyramid (supported now) or image arrays (not implemented yet) */
                if (in0->base.scope->type == VX_TYPE_PYRAMID && in1->base.scope->type == VX_TYPE_PYRAMID &&
                    output->base.scope->type == VX_TYPE_PYRAMID)
                {
                    vx_size pyr0_levels = 0;
                    vx_size pyr1_levels = 0;
                    vx_size pyr3_levels = 0;

                    vx_pyramid pyr0 = (vx_pyramid)in0->base.scope;
                    vx_pyramid pyr1 = (vx_pyramid)in1->base.scope;
                    vx_pyramid pyr3 = (vx_pyramid)output->base.scope;

                    status = vxQueryPyramid(pyr0, VX_PYRAMID_LEVELS, &pyr0_levels, sizeof(pyr0_levels));
                    if (VX_SUCCESS != status)
                        return status;

                    status = vxQueryPyramid(pyr1, VX_PYRAMID_LEVELS, &pyr1_levels, sizeof(pyr1_levels));
                    if (VX_SUCCESS != status)
                        return status;

                    status = vxQueryPyramid(pyr3, VX_PYRAMID_LEVELS, &pyr3_levels, sizeof(pyr3_levels));
                    if (VX_SUCCESS != status)
                        return status;

                    if (pyr0_levels != pyr1_levels || pyr0_levels != pyr3_levels)
                        return VX_FAILURE;

                    for (i = 0; i < pyr3_levels; i++)
                    {
                        vx_image src0 = vxGetPyramidLevel(pyr0, i);
                        vx_image src1 = vxGetPyramidLevel(pyr1, i);
                        vx_image dst  = vxGetPyramidLevel(pyr3, i);

                        status = vxSubtraction(node, src0, src1, policy_param, dst);

                        status |= vxReleaseImage(&src0);
                        status |= vxReleaseImage(&src1);
                        status |= vxReleaseImage(&dst);
                    }
                }
            }
        }
        else
            status = vxSubtraction(node, in0, in1, policy_param, output);

        return status;
    }

    return VX_ERROR_INVALID_PARAMETERS;
}

vx_kernel_description_t subtract_kernel = {
	VX_KERNEL_SUBTRACT,
	"org.khronos.openvx.subtract",
	vxSubtractionKernel,
	add_subtract_kernel_params, dimof(add_subtract_kernel_params),
	NULL,
	vxAddSubtractInputValidator,
	vxAddSubtractOutputValidator,	
	NULL,
    NULL,
};
