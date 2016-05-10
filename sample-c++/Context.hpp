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

#ifndef _VX_CONTEXT_HPP_
#define _VX_CONTEXT_HPP_

#include <vx.hpp>

namespace OpenVX {

    /*! \brief A Singleton Context Object for OpenVX. */
    class VX_CLASS Context  {

        vx_context m_context;

        vx_char  m_implementationName[VX_MAX_IMPLEMENTATION_NAME];
        vx_char* m_extensions;

    public:
        Context();    // vxCreateContext
        ~Context();   // vxReleaseContext

        vx_context context() { return m_context; }

        vx_status setImmediateBorderMode(vx_border_t *config);     // vxSetImmediateBorderMode
        vx_status hint(vx_reference reference, vx_enum hint, const void* data, vx_size data_size);           // vxHint
        vx_status directive(vx_reference reference, vx_enum directive); // vxDirective
        vx_status loadKernels(const char *filename);

        // vxQueryContext
        vx_uint16 vendorID();
        vx_uint16 version();
        vx_uint32 numKernels();
        vx_uint32 numModules();
        vx_uint32 numRefs();
#if defined(EXPERIMENTAL_USE_TARGET)
        vx_uint32 numTargets();
#endif
        const char* implementation();
        vx_size extensionSize();
        const char *extensions();
        vx_size convolutionMaxDimension();

        // object creation (ordered as in forward declarations in vx.hpp)
        Array*                           createArray(vx_enum item_type, vx_size capacity)                   { return (new Array(this, item_type, capacity)); }
        Convolution*                     createConvolution(vx_size cols, vx_size rows)                      { return (new Convolution(this, cols, rows)); }
        Distribution*                    createDistribution(vx_size numBins, vx_size offset, vx_size range) { return (new Distribution(this, numBins, offset, range)); }
        Graph*                           createGraph()                                                      { return (new Graph(this)); }
        //Image*                           createImage()                                                      { return (new Image(this)); }
        //Image*                           createImage(vx_df_image color)                                       { return (new Image(this, color)); }
        Image*                           createImage(vx_uint32 width, vx_uint32 height, vx_df_image color)    { return (new Image(this, width, height, color)); }
        Image*                           createImage(vx_df_image color, vx_imagepatch_addressing_t addrs[],
                                                     void* ptrs[], vx_enum type)                            { return (new Image(this, color, addrs, ptrs, type)); }
        Image*                           createImage(vx_uint32 width, vx_uint32 height,
                                                     vx_df_image color, vx_pixel_value_t* value)            { return (new Image(this, width, height, color, value)); }
        template <class T> LUT<T>*       createLUT(vx_size count)                                           { return (new LUT<T>(this, count)); }
        template <class T> Matrix<T>*    createMatrix(vx_size cols, vx_size rows)                           { return (new Matrix<T>(this, cols, rows)); };
        Pyramid*                         createPyramid(vx_size levels, vx_float32 scale,
                                                       vx_uint32 width, vx_uint32 height, vx_df_image format) { return (new Pyramid(this, levels, scale, width, height, format)); }
        Remap*                           createRemap(vx_uint32 src_width, vx_uint32 src_height,
                                                     vx_uint32 dst_width, vx_uint32 dst_height)             { return (new Remap(this, src_width, src_height, dst_width, dst_height)); }
        String*                          createString(const vx_char* pString)                                     { return (new String(this, pString)); }
#if defined(EXPERIMENTAL_USE_TARGET)
        Target*                          createTarget(vx_uint32 index)                                      { return (new Target(this, index)); }
        Target*                          createTarget(const vx_char* name)                                  { return (new Target(this, name)); }
#endif
        template <class T> Threshold<T>* createThreshold (vx_enum thresh_type)                              { return (new Threshold<T>(this, thresh_type)); }
        Delay<Image>*                    createDelay(vx_uint32 width, vx_uint32 height,
                                                     vx_df_image format, vx_size num);                        // can't inline for reasons unknown (confounded specialized template esoterica)

        Char*                            createChar(vx_char v){return (new Char(this, v));}
        Bool*                            createBool(vx_bool v){return (new Bool(this, v));}
        Size*                            createSize(vx_size v){return (new Size(this, v));}
        FourCC*                          createFourCC(vx_df_image v){return (new FourCC(this, v));}
        Enumerant*                       createEnumerant(vx_enum v){return (new Enumerant(this, v));}
        UInt8*                           createBool(vx_uint8 v){return (new UInt8(this, v));}
        Int8*                            createBool(vx_int8 v){return (new Int8(this, v));}
        UInt16*                          createBool(vx_uint16 v){return (new UInt16(this, v));}
        Int16*                           createBool(vx_int16 v){return (new Int16(this, v));}
        UInt32*                          createBool(vx_uint32 v){return (new UInt32(this, v));}
        Int32*                           createBool(vx_int32 v){return (new Int32(this, v));}
        UInt64*                          createBool(vx_uint64 v){return (new UInt64(this, v));}
        Int64*                           createBool(vx_int64 v){return (new Int64(this, v));}
        Float32*                         createBool(vx_float32 v){return (new Float32(this, v));}
        Float64*                         createBool(vx_float64 v){return (new Float64(this, v));}
    };
}

#endif

