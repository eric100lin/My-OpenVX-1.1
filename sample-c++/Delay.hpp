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

#ifndef _VX_DELAY_HPP_
#define _VX_DELAY_HPP_

#include <vx.hpp>

namespace OpenVX {

    template <class T>
    class VX_CLASS Delay : public Parameter {

        friend class Context;

    protected:
        Delay(Context* pContext, vx_uint32 width, vx_uint32 height, vx_df_image format, vx_size num);  // T == Image only

    public:
        ~Delay();  // vxReleaseDelay

        T *operator[](vx_uint32 index);

        vx_status associate(vx_int32 delay_index, Node* pNode, vx_uint32 param_index, vx_enum param_direction);  // vxAssociateDelayWithNode
        vx_status dissociate(vx_int32 delay_index, Node* pNode, vx_uint32 param_index);                          // vxDissociateDelayFromNode
        vx_status age();  // vxAgeDelay

        // vxQueryDelay
        vx_enum type();
        vx_size count();
    };

};

#endif

