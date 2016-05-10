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
#include <Matrix.hpp>

namespace OpenVX {

// constructors need to be specialized
template <> Matrix<vx_char>    :: Matrix(Context* pContext, vx_size cols, vx_size rows) : Parameter(pContext, (vx_reference)vxCreateMatrix(pContext->context(), VX_TYPE_CHAR,    cols, rows)) {}
template <> Matrix<vx_uint8>   :: Matrix(Context* pContext, vx_size cols, vx_size rows) : Parameter(pContext, (vx_reference)vxCreateMatrix(pContext->context(), VX_TYPE_UINT8,   cols, rows)) {}
template <> Matrix<vx_int8>    :: Matrix(Context* pContext, vx_size cols, vx_size rows) : Parameter(pContext, (vx_reference)vxCreateMatrix(pContext->context(), VX_TYPE_INT8,    cols, rows)) {}
template <> Matrix<vx_uint16>  :: Matrix(Context* pContext, vx_size cols, vx_size rows) : Parameter(pContext, (vx_reference)vxCreateMatrix(pContext->context(), VX_TYPE_UINT16,  cols, rows)) {}
template <> Matrix<vx_int16>   :: Matrix(Context* pContext, vx_size cols, vx_size rows) : Parameter(pContext, (vx_reference)vxCreateMatrix(pContext->context(), VX_TYPE_INT16,   cols, rows)) {}
template <> Matrix<vx_uint32>  :: Matrix(Context* pContext, vx_size cols, vx_size rows) : Parameter(pContext, (vx_reference)vxCreateMatrix(pContext->context(), VX_TYPE_UINT32,  cols, rows)) {}
template <> Matrix<vx_int32>   :: Matrix(Context* pContext, vx_size cols, vx_size rows) : Parameter(pContext, (vx_reference)vxCreateMatrix(pContext->context(), VX_TYPE_INT32,   cols, rows)) {}
template <> Matrix<vx_uint64>  :: Matrix(Context* pContext, vx_size cols, vx_size rows) : Parameter(pContext, (vx_reference)vxCreateMatrix(pContext->context(), VX_TYPE_UINT64,  cols, rows)) {}
template <> Matrix<vx_int64>   :: Matrix(Context* pContext, vx_size cols, vx_size rows) : Parameter(pContext, (vx_reference)vxCreateMatrix(pContext->context(), VX_TYPE_INT64,   cols, rows)) {}
template <> Matrix<vx_float32> :: Matrix(Context* pContext, vx_size cols, vx_size rows) : Parameter(pContext, (vx_reference)vxCreateMatrix(pContext->context(), VX_TYPE_FLOAT32, cols, rows)) {}
template <> Matrix<vx_float64> :: Matrix(Context* pContext, vx_size cols, vx_size rows) : Parameter(pContext, (vx_reference)vxCreateMatrix(pContext->context(), VX_TYPE_FLOAT64, cols, rows)) {}

template <class T> Matrix<T>::~Matrix()
{
    vxReleaseMatrix((vx_matrix *)&m_handle);
}

#ifdef VX_1_0_1_NAMING_COMPATIBILITY
template <class T> void Matrix<T>::Access(T* mat)
{
    return vxReadMatrix(handle(), mat);
}

template <class T> void Matrix<T>::Commit(T* mat)
{
    return vxWriteMatrix(handle(), mat);
}
#endif

template <class T> vx_status Matrix<T>::Read(T* data)
{
    return vxCopyMatrix((vx_matrix)handle(), data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
}

template <class T> vx_status Matrix<T>::Write(T* data)
{
    return vxCopyMatrix((vx_matrix)handle(), data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
}

// vxQueryMatrix
template <class T> vx_enum Matrix<T>::type()
{
    vx_enum v;
    vxQueryMatrix(handle(), VX_MATRIX_TYPE, &v, sizeof(v));
    return v;
}

template <class T> vx_size Matrix<T>::rows()
{
    vx_size v;
    vxQueryMatrix(handle(), VX_MATRIX_ROWS, &v, sizeof(v));
    return v;
}

template <class T> vx_size Matrix<T>::columns()
{
    vx_size v;
    vxQueryMatrix(handle(), VX_MATRIX_COLUMNS, &v, sizeof(v));
    return v;
}

template <class T> vx_size Matrix<T>::size()
{
    vx_size v;
    vxQueryMatrix(handle(), VX_MATRIX_SIZE, &v, sizeof(v));
    return v;
}

}

