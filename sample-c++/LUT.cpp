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

GenericLUT::GenericLUT(Context* pContext, vx_size count, vx_enum type) : Parameter(pContext, (vx_reference)vxCreateLUT(pContext->context(), type, count)) {}

// constructors need to be specialized
//template <> LUT<vx_char>    :: LUT(Context* pContext, vx_size count) : GenericLUT(pContext, count, VX_TYPE_CHAR   ) {}
  template <> LUT<vx_uint8>   :: LUT(Context* pContext, vx_size count) : GenericLUT(pContext, count, VX_TYPE_UINT8  ) {}
//template <> LUT<vx_int8>    :: LUT(Context* pContext, vx_size count) : GenericLUT(pContext, count, VX_TYPE_INT8   ) {}
//template <> LUT<vx_uint16>  :: LUT(Context* pContext, vx_size count) : GenericLUT(pContext, count, VX_TYPE_UINT16 ) {}
//template <> LUT<vx_int16>   :: LUT(Context* pContext, vx_size count) : GenericLUT(pContext, count, VX_TYPE_INT16  ) {}
//template <> LUT<vx_uint32>  :: LUT(Context* pContext, vx_size count) : GenericLUT(pContext, count, VX_TYPE_UINT32 ) {}
//template <> LUT<vx_int32>   :: LUT(Context* pContext, vx_size count) : GenericLUT(pContext, count, VX_TYPE_INT32  ) {}
//template <> LUT<vx_uint64>  :: LUT(Context* pContext, vx_size count) : GenericLUT(pContext, count, VX_TYPE_UINT64 ) {}
//template <> LUT<vx_int64>   :: LUT(Context* pContext, vx_size count) : GenericLUT(pContext, count, VX_TYPE_INT64  ) {}
//template <> LUT<vx_float32> :: LUT(Context* pContext, vx_size count) : GenericLUT(pContext, count, VX_TYPE_FLOAT32) {}
//template <> LUT<vx_float64> :: LUT(Context* pContext, vx_size count) : GenericLUT(pContext, count, VX_TYPE_FLOAT64) {}

template <class T> LUT<T>::~LUT()
{
    vxReleaseLUT((vx_lut *)&m_handle);
}

template <class T> void LUT<T>::Access(T **lut)
{
    return vxAccessLUT(handle(), lut);
}

template <class T> void LUT<T>::Commit(T *lut)
{
    return vxCommitLUT(handle(), lut);
}

template <class T> vx_enum LUT<T>::type()
{
    vx_enum stype;
    vxQueryLUT(handle(), VX_SCALAR_TYPE, &stype, sizeof(stype));
    return stype;
}

template <class T> vx_size LUT<T>::count()
{
    vx_size v;
    vxQueryLUT(handle(), VX_LUT_COUNT, &v, sizeof(v));
    return v;
}

template <class T> vx_size LUT<T>::size()
{
    vx_size v;
    vxQueryLUT(handle(), VX_LUT_SIZE, &v, sizeof(v));
    return v;
}

}

