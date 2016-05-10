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

Pyramid::Pyramid(Context* pContext,
                 vx_size levels, vx_float32 scale,
                 vx_uint32 width, vx_uint32 height,
                 vx_df_image format) :
         Parameter(pContext, (vx_reference)vxCreatePyramid(pContext->context(), levels, scale, width, height, format))
{}

Pyramid::~Pyramid()
{
    vxReleasePyramid((vx_pyramid *)&m_handle);
}

Image* Pyramid::getLevel(vx_uint32 index)
{
    vx_image img = vxGetPyramidLevel((vx_pyramid)handle(), index);
    return (img ? Image::wrap(context(),img) : NULL);
}

// vxQueryPyramid
vx_size Pyramid::levels()
{
    vx_size v;
    vxQueryPyramid((vx_pyramid)handle(), VX_PYRAMID_LEVELS, &v, sizeof(v));
    return v;
}

vx_float32 Pyramid::scale()
{
    vx_float32 v;
    vxQueryPyramid((vx_pyramid)handle(), VX_PYRAMID_SCALE, &v, sizeof(v));
    return v;
}

