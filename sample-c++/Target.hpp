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

#ifndef _VX_TARGET_HPP_
#define _VX_TARGET_HPP_

#include <vx.hpp>

#if defined(EXPERIMENTAL_USE_TARGET)

// BUG BUG do we need to support this? Does Target serve any function
// besides querying kernels per target which we don't intend to support?

namespace OpenVX {

    class VX_CLASS Target /*: public Reference */ {

        friend class Context;

    protected:
        vx_uint32           m_targetIndex;
        vx_char             m_targetName[VX_MAX_TARGET_NAME];
        vx_uint32           m_numKernels;
        vx_kernel_info_t* m_pKernelTable;

        Target(Context* pContext, vx_uint32 index);      // vxGetTargetByIndex
        Target(Context* pContext, const vx_char *name);  // vxGetTargetByName

    public:
        virtual ~Target();            // vxReleaseTarget

        // BUG BUG this is all hideously broken. need implementation
        // that doesn't rely on underyling C which demands a reference
        vx_status AssignNode(Node* pNode);  // vxAssignNode

        vx_uint32   index();
        const char* name();
        vx_uint32   numKernels();
        vx_kernel_info_t* kernelTable();
    };
};

#endif

#endif

