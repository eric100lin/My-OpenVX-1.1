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
#include <stdlib.h>

using namespace OpenVX;

#if defined(EXPERIMENTAL_USE_TARGET)

Target::Target(Context* pContext, vx_uint32 idx) //:
        //Reference(pContext, vxGetTargetByIndex(pContext->context(), idx))
{
    m_pKernelTable = NULL;
}

Target::Target(Context* pContext, const vx_char *name) //:
        //Reference(pContext, vxGetTargetByName(pContext->context(), name))
{
    m_pKernelTable = NULL;
}

Target::~Target()
{
    if (m_pKernelTable)
    {
        free(m_pKernelTable);
    }
}

vx_status Target::AssignNode(Node* pNode)
{
    return 0;
    //vxAssignNode(pNode->handle(), handle());
}

vx_uint32 Target::index()
{
    return m_targetIndex;
}

const char* Target::name()
{
    return m_targetName;
}

vx_uint32 Target::numKernels()
{
    return m_numKernels;
}

vx_kernel_info_t *Target::kernelTable()
{
    return m_pKernelTable;
}

#endif
