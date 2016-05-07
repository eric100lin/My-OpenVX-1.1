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

// nodeless version of the Magnitude kernel
vx_status vxMagnitude(vx_image grad_x, vx_image grad_y, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_uint32 y, x;
    vx_df_image format = 0;
    vx_uint8 *dst_base   = NULL;
    vx_int16 *src_base_x = NULL;
    vx_int16 *src_base_y = NULL;
    vx_imagepatch_addressing_t dst_addr, src_addr_x, src_addr_y;
    vx_rectangle_t rect;
    vx_uint32 value;

    if (grad_x == 0 || grad_y == 0)
        return VX_ERROR_INVALID_PARAMETERS;

    vxQueryImage(output, VX_IMAGE_FORMAT, &format, sizeof(format));
    status = vxGetValidRegionImage(grad_x, &rect);
    status |= vxAccessImagePatch(grad_x, &rect, 0, &src_addr_x, (void **)&src_base_x, VX_READ_ONLY);
    status |= vxAccessImagePatch(grad_y, &rect, 0, &src_addr_y, (void **)&src_base_y, VX_READ_ONLY);
    status |= vxAccessImagePatch(output, &rect, 0, &dst_addr, (void **)&dst_base, VX_WRITE_ONLY);
    for (y = 0; y < src_addr_x.dim_y; y++)
    {
        for (x = 0; x < src_addr_x.dim_x; x++)
        {
            vx_int16 *in_x = vxFormatImagePatchAddress2d(src_base_x, x, y, &src_addr_x);
            vx_int16 *in_y = vxFormatImagePatchAddress2d(src_base_y, x, y, &src_addr_y);
            if (format == VX_DF_IMAGE_U8)
            {
                vx_uint8 *dst = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                vx_int32 grad[2] = {in_x[0]*in_x[0], in_y[0]*in_y[0]};
                vx_float64 sum = grad[0] + grad[1];
                value = ((vx_int32)sqrt(sum))/4;
                *dst = (vx_uint8)(value > UINT8_MAX ? UINT8_MAX : value);
            }
            else if (format == VX_DF_IMAGE_S16)
            {
                vx_uint16 *dst = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                vx_float64 grad[2] = {(vx_float64)in_x[0]*in_x[0], (vx_float64)in_y[0]*in_y[0]};
                vx_float64 sum = grad[0] + grad[1];
                value = (vx_int32)(sqrt(sum) + 0.5);
                *dst = (vx_int16)(value > INT16_MAX ? INT16_MAX : value);
            }
        }
    }
    status |= vxCommitImagePatch(grad_x, NULL, 0, &src_addr_x, src_base_x);
    status |= vxCommitImagePatch(grad_y, NULL, 0, &src_addr_y, src_base_y);
    status |= vxCommitImagePatch(output, &rect, 0, &dst_addr, dst_base);
    return status;
}

