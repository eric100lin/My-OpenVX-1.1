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

#include <vx.hpp>

using namespace OpenVX;

/*
Image::Image(Context* pContext) :
       Parameter(pContext, (vx_reference)vxCreateVirtualImage(pContext->context()))
{}

Image::Image(Context* pContext, vx_df_image color) :
       Parameter(pContext, (vx_reference)vxCreateVirtualImageWithFormat(pContext->context(), color))
{}
*/

Image::Image(Context* pContext,
             vx_uint32 width,
             vx_uint32 height,
             vx_df_image color) :
       Parameter(pContext, (vx_reference)vxCreateImage(pContext->context(), width, height, color))
{}

Image::Image(Context* pContext,
             vx_df_image color,
             vx_imagepatch_addressing_t addrs[],
             void* ptrs[],
             vx_enum type) :
       Parameter(pContext, (vx_reference)vxCreateImageFromHandle(pContext->context(), color, addrs, ptrs, type))
{}

Image::Image(Image* pImg,
             vx_rectangle_t *rect) :
       Parameter(pImg->context(), (vx_reference)vxCreateImageFromROI((vx_image)pImg->handle(), rect))
{}

Image::Image(Context* pContext,
             vx_uint32 width,
             vx_uint32 height,
             vx_df_image color,
             vx_pixel_value_t* value) :
       Parameter(pContext, (vx_reference)vxCreateUniformImage(pContext->context(), width, height, color, value))
{}

Image::~Image()
{
    vxReleaseImage((vx_image *)&m_handle);
}

vx_status Image::AccessImagePatch(vx_rectangle_t *rect, vx_uint32 p, vx_imagepatch_addressing_t *addr, void **ptr, vx_enum usage)
{
    return vxAccessImagePatch((vx_image)handle(), rect, p, addr, ptr, usage);
}

vx_status Image::CommitImagePatch(vx_rectangle_t *rect, vx_uint32 p, vx_imagepatch_addressing_t *addr, void *ptr)
{
    return vxCommitImagePatch((vx_image)handle(), rect, p, addr, ptr);
}

vx_size Image::ComputeImagePatchSize(vx_rectangle_t *rect, vx_uint32 p)
{
    return vxComputeImagePatchSize((vx_image)handle(), rect, p);
}

void* Image::FormatImagePatchAddress1d(void *ptr, vx_uint32 index, vx_imagepatch_addressing_t* addr)
{
    return vxFormatImagePatchAddress1d(ptr, index, addr);
}

void* Image::FormatImagePatchAddress2d(void *ptr, vx_uint32 x, vx_uint32 y, vx_imagepatch_addressing_t* addr)
{
    return vxFormatImagePatchAddress2d(ptr, x, y, addr);
}

vx_rectangle_t Image::GetValidRegionImage(Image* pImage)
{
    vx_rectangle_t rect = {0, 0, 0, 0};
    vxGetValidRegionImage((vx_image)handle(), &rect);
    return rect;
}

vx_uint32 Image::width()
{
    vx_uint32 v;
    vxQueryImage((vx_image)handle(), VX_IMAGE_WIDTH, &v, sizeof(v));
    return v;
}

vx_uint32 Image::height()
{
    vx_uint32 v;
    vxQueryImage((vx_image)handle(), VX_IMAGE_HEIGHT, &v, sizeof(v));
    return v;
}

vx_df_image Image::format()
{
    vx_df_image v;
    vxQueryImage((vx_image)handle(), VX_IMAGE_FORMAT, &v, sizeof(v));
    return v;
}

vx_enum Image::range()
{
    vx_enum v;
    vxQueryImage((vx_image)handle(), VX_IMAGE_RANGE, &v, sizeof(v));
    return v;
}

vx_enum Image::space()
{
    vx_enum v;
    vxQueryImage((vx_image)handle(), VX_IMAGE_SPACE, &v, sizeof(v));
    return v;
}

vx_uint32 Image::planes()
{
    vx_size v;
    vxQueryImage((vx_image)handle(), VX_IMAGE_PLANES, &v, sizeof(v));
    return v;
}



