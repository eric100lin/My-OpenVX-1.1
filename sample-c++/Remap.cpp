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

Remap::Remap(Context* pContext,
             vx_uint32 src_width, vx_uint32 src_height,
             vx_uint32 dst_width, vx_uint32 dst_height) :
       Parameter(pContext, (vx_reference)vxCreateRemap(pContext->context(), src_width, src_height, dst_width, dst_height))
{}


// vxCreateRemap
Remap::~Remap()
{
    vxReleaseRemap((vx_remap *)&m_handle);
}

vx_status Remap::vxSetPoint(vx_uint32 dst_x, vx_uint32 dst_y,
                            vx_float32 src_x, vx_float32 src_y)
{
    return vxSetRemapPoint((vx_remap)handle(), dst_x, dst_y, src_x, src_y);
}

vx_status Remap::vxGetPoint(vx_uint32 dst_x, vx_uint32 dst_y,
                            vx_float32 *src_x, vx_float32 *src_y)
{
    return vxGetRemapPoint((vx_remap)handle(), dst_x, dst_y, src_x, src_y);
}

vx_uint32 Remap::sourceWidth()
{
    vx_uint32 v;
    vxQueryRemap((vx_remap)handle(), VX_REMAP_SOURCE_WIDTH, &v, sizeof(v));
    return v;
}

vx_uint32 Remap::sourceHeight()
{
    vx_uint32 v;
    vxQueryRemap((vx_remap)handle(), VX_REMAP_SOURCE_HEIGHT, &v, sizeof(v));
    return v;
}

vx_uint32 Remap::destWidth()
{
    vx_uint32 v;
    vxQueryRemap((vx_remap)handle(), VX_REMAP_DESTINATION_WIDTH, &v, sizeof(v));
    return v;
}

vx_uint32 Remap::destHeight()
{
    vx_uint32 v;
    vxQueryRemap((vx_remap)handle(), VX_REMAP_DESTINATION_HEIGHT, &v, sizeof(v));
    return v;
}

