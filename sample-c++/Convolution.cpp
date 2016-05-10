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

Convolution::Convolution(Context* pContext, vx_size cols, vx_size rows) :
             Parameter(pContext, (vx_reference)vxCreateConvolution(pContext->context(), cols, rows))
{}

Convolution::~Convolution()
{
    vxReleaseConvolution((vx_convolution *)&m_handle);
}

vx_status Convolution::setAttribute(vx_enum attr, void *ptr, vx_size size)
{
    return vxSetConvolutionAttribute((vx_convolution)handle(), attr, ptr, size);
}

#ifdef VX_1_0_1_NAMING_COMPATIBILITY
vx_status Convolution::accessCoefficients(vx_int16 *coeff)
{
    return vxReadConvolutionCoefficients((vx_convolution)handle(), coeff);
}

vx_status Convolution::commitCoefficients(vx_int16 *coeff)
{
    return vxWriteConvolutionCoefficients((vx_convolution)handle(), coeff);
}
#endif

vx_status Convolution::readCoefficients(vx_int16 *coeff)
{
    return vxCopyConvolutionCoefficients((vx_convolution)handle(), coeff, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
}

vx_status Convolution::writeCoefficients(vx_int16 *coeff)
{
    return vxCopyConvolutionCoefficients((vx_convolution)handle(), coeff, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
}

vx_size Convolution::rows()
{
    vx_size v;
    vxQueryConvolution((vx_convolution)handle(), VX_CONVOLUTION_ROWS, &v, sizeof(v));
    return v;
}

vx_size Convolution::columns()
{
    vx_size v;
    vxQueryConvolution((vx_convolution)handle(), VX_CONVOLUTION_COLUMNS, &v, sizeof(v));
    return v;
}

vx_uint32 Convolution::scale()
{
    vx_uint32 v;
    vxQueryConvolution((vx_convolution)handle(), VX_CONVOLUTION_SCALE, &v, sizeof(v));
    return v;
}

vx_size Convolution::size()
{
    vx_size v;
    vxQueryConvolution((vx_convolution)handle(), VX_CONVOLUTION_SIZE, &v, sizeof(v));
    return v;
}

