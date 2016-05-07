/*
 * Copyright (c) 2012-2014 The Khronos Group Inc.
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

#include <c_model.h>
#include <vx_debug.h>

// nodeless version of the Threshold kernel
vx_status vxThreshold(vx_image src_image, vx_threshold threshold, vx_image dst_image)
{
    vx_enum type = 0;
    vx_rectangle_t rect;
    vx_imagepatch_addressing_t src_addr;
    vx_imagepatch_addressing_t dst_addr;
    void *src_base = NULL;
    void *dst_base = NULL;
    vx_uint32 y = 0;
    vx_uint32 x = 0;
    vx_int32 value = 0;
    vx_int32 lower = 0;
    vx_int32 upper = 0;
    vx_int32 true_value;
    vx_int32 false_value;
    vx_status status = VX_FAILURE;

    vxQueryThreshold(threshold, VX_THRESHOLD_TYPE, &type, sizeof(type));

    if (type == VX_THRESHOLD_TYPE_BINARY)
    {
        vxQueryThreshold(threshold, VX_THRESHOLD_THRESHOLD_VALUE, &value, sizeof(value));
        VX_PRINT(VX_ZONE_INFO, "threshold type binary, value = %u\n", value);
    }
    else if (type == VX_THRESHOLD_TYPE_RANGE)
    {
        vxQueryThreshold(threshold, VX_THRESHOLD_THRESHOLD_LOWER, &lower, sizeof(lower));
        vxQueryThreshold(threshold, VX_THRESHOLD_THRESHOLD_UPPER, &upper, sizeof(upper));
        VX_PRINT(VX_ZONE_INFO, "threshold type range, lower = %u, upper = %u\n", lower, upper);
    }

    vxQueryThreshold(threshold, VX_THRESHOLD_TRUE_VALUE, &true_value, sizeof(true_value));
    vxQueryThreshold(threshold, VX_THRESHOLD_FALSE_VALUE, &false_value, sizeof(false_value));
    VX_PRINT(VX_ZONE_INFO, "threshold true value = %u, threshold false value = %u\n", true_value, false_value);

    status = vxGetValidRegionImage(src_image, &rect);
    status |= vxAccessImagePatch(src_image, &rect, 0, &src_addr, &src_base, VX_READ_ONLY);
    status |= vxAccessImagePatch(dst_image, &rect, 0, &dst_addr, &dst_base, VX_WRITE_ONLY);

    for (y = 0; y < src_addr.dim_y; y++)
    {
        for (x = 0; x < src_addr.dim_x; x++)
        {
            vx_uint8 *src_ptr = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
            vx_uint8 *dst_ptr = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);

            if (type == VX_THRESHOLD_TYPE_BINARY)
            {
                if (*src_ptr > value)
                    *dst_ptr = (vx_uint8)true_value;
                else
                    *dst_ptr = (vx_uint8)false_value;
            }
            if (type == VX_THRESHOLD_TYPE_RANGE)
            {
                if (*src_ptr > upper)
                    *dst_ptr = (vx_uint8)false_value;
                else if (*src_ptr < lower)
                    *dst_ptr = (vx_uint8)false_value;
                else
                    *dst_ptr = (vx_uint8)true_value;
            }
        }
    }

    status |= vxCommitImagePatch(src_image, NULL, 0, &src_addr, src_base);
    status |= vxCommitImagePatch(dst_image, &rect, 0, &dst_addr, dst_base);

    return status;
}

