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

// constructors need to be specialized
// only uint8 is legal in VX 1.0
//template <> Threshold<vx_char>    :: Threshold(Context* pContext, vx_enum thresh_type) : Parameter(pContext, (vx_reference)vxCreateThreshold(pContext->context(), thresh_type, VX_TYPE_CHAR   )) {}
template <> Threshold<vx_uint8>   :: Threshold(Context* pContext, vx_enum thresh_type) : Parameter(pContext, (vx_reference)vxCreateThreshold(pContext->context(), thresh_type, VX_TYPE_UINT8  )) {}
//template <> Threshold<vx_int8>    :: Threshold(Context* pContext, vx_enum thresh_type) : Parameter(pContext, (vx_reference)vxCreateThreshold(pContext->context(), thresh_type, VX_TYPE_INT8   )) {}
//template <> Threshold<vx_uint16>  :: Threshold(Context* pContext, vx_enum thresh_type) : Parameter(pContext, (vx_reference)vxCreateThreshold(pContext->context(), thresh_type, VX_TYPE_UINT16 )) {}
//template <> Threshold<vx_int16>   :: Threshold(Context* pContext, vx_enum thresh_type) : Parameter(pContext, (vx_reference)vxCreateThreshold(pContext->context(), thresh_type, VX_TYPE_INT16  )) {}
//template <> Threshold<vx_uint32>  :: Threshold(Context* pContext, vx_enum thresh_type) : Parameter(pContext, (vx_reference)vxCreateThreshold(pContext->context(), thresh_type, VX_TYPE_UINT32 )) {}
//template <> Threshold<vx_int32>   :: Threshold(Context* pContext, vx_enum thresh_type) : Parameter(pContext, (vx_reference)vxCreateThreshold(pContext->context(), thresh_type, VX_TYPE_INT32  )) {}
//template <> Threshold<vx_uint64>  :: Threshold(Context* pContext, vx_enum thresh_type) : Parameter(pContext, (vx_reference)vxCreateThreshold(pContext->context(), thresh_type, VX_TYPE_UINT64 )) {}
//template <> Threshold<vx_int64>   :: Threshold(Context* pContext, vx_enum thresh_type) : Parameter(pContext, (vx_reference)vxCreateThreshold(pContext->context(), thresh_type, VX_TYPE_INT64  )) {}
//template <> Threshold<vx_float32> :: Threshold(Context* pContext, vx_enum thresh_type) : Parameter(pContext, (vx_reference)vxCreateThreshold(pContext->context(), thresh_type, VX_TYPE_FLOAT32)) {}
//template <> Threshold<vx_float64> :: Threshold(Context* pContext, vx_enum thresh_type) : Parameter(pContext, (vx_reference)vxCreateThreshold(pContext->context(), thresh_type, VX_TYPE_FLOAT64)) {}

template <class T> Threshold<T>::~Threshold()
{
    vxReleaseThreshold((vx_threshold *)&m_handle);
}

template <class T> vx_status Threshold<T>::SetAttribute(vx_enum attribute, void *ptr, vx_size size)
{
    return vxSetThresholdAttribute(handle(), attribute, ptr, size);
}

// vxQueryThreshold
template <class T> vx_enum Threshold<T>::type()
{
    vx_enum v;
    vxQueryThreshold(handle(), VX_THRESHOLD_TYPE, &v, sizeof(v));
    return v;
}

template <class T> vx_uint8 Threshold<T>::value()
{
    vx_int32 v;
    vxQueryThreshold(handle(), VX_THRESHOLD_THRESHOLD_VALUE, &v, sizeof(v));
    return v;
}

template <class T> vx_uint8 Threshold<T>::lower()
{
    vx_int32 v;
    vxQueryThreshold(handle(), VX_THRESHOLD_THRESHOLD_LOWER, &v, sizeof(v));
    return v;
}

template <class T> vx_uint8 Threshold<T>::upper()
{
    vx_int32 v;
    vxQueryThreshold(handle(), VX_THRESHOLD_THRESHOLD_UPPER, &v, sizeof(v));
    return v;
}

}

