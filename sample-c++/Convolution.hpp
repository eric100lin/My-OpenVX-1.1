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

#ifndef _VX_CONVOLUTION_HPP_
#define _VX_CONVOLUTION_HPP_

namespace OpenVX {

    class VX_CLASS Convolution : public Parameter {

        friend class Context;

    protected:
        Convolution(Context* pContext, vx_size cols, vx_size rows);  // vxCreateConvolution
        //Convolution(Convolution* pConv);

    public:
        ~Convolution();     // vxReleaseConvolution

        vx_status setAttribute(vx_enum attr, void *ptr, vx_size size);  // vxSetConvolutionAttribute

#ifdef VX_1_0_1_NAMING_COMPATIBILITY
        vx_status accessCoefficients(vx_int16 *coeff);   // vxReadConvolutionCoefficients
        vx_status commitCoefficients(vx_int16 *coeff);   // vxWriteConvolutionCoefficients
#endif
        vx_status readCoefficients(vx_int16 *coeff);    // vxCopyConvolutionCoefficients
        vx_status writeCoefficients(vx_int16 *coeff);   // vxCopyConvolutionCoefficients

        // vxQueryConvolution
        vx_size   rows();
        vx_size   columns();
        vx_uint32 scale();
        vx_size   size();
    };

};

#endif

