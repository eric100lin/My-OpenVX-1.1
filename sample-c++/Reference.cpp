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

Reference::Reference(Context* pContext)
{
    m_pContext = pContext;
    m_handle = 0;
}

Reference::Reference(Context* pContext, vx_reference ref)
{
    m_pContext = pContext;
    m_handle = ref;
}

Reference::~Reference()
{
    m_handle = 0;
}

Context* Reference::context()
{
    return m_pContext;
}

vx_reference Reference::handle()
{
    return m_handle;
}

vx_enum Reference::type()
{
    vx_enum rtype = VX_TYPE_INVALID;
    vxQueryReference(m_handle, VX_REF_ATTRIBUTE_TYPE, &rtype, sizeof(rtype));
    return rtype;
}

vx_uint32 Reference::count()
{
    vx_uint32 count = 0;
    vxQueryReference(m_handle, VX_REF_ATTRIBUTE_COUNT, &count, sizeof(count));
    return count;
}

