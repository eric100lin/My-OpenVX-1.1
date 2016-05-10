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

namespace OpenVX {

Scalar::Scalar(Context *pContext, vx_enum type, void *v)
        :Parameter(pContext, (vx_reference)vxCreateScalar(pContext->context(), type, v))
{}

Scalar::~Scalar()
{
    vxReleaseScalar((vx_scalar *)&m_handle);
}

vx_enum Scalar::type()
{
    vx_enum stype;
    vxQueryScalar((vx_scalar)handle(), VX_SCALAR_TYPE, &stype, sizeof(stype));
    return stype;
}

vx_char Char::AccessValue()
{
    vx_char v;
    vxCopyScalar((vx_scalar)handle(), &v, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
    return v;
}

void Char::CommitValue(vx_char v)
{
    vxCopyScalar((vx_scalar)handle(), &v, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
}

}

