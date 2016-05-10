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
/*
Array::Array(Graph* pGraph, vx_enum item_type, vx_size capacity) :
        Parameter(pGraph->m_pContext, (vx_reference)vxCreateVirtualArray(0, 0, 0))
{}
*/
Array::Array(Context* pContext, vx_enum item_type, vx_size capacity) :
    Parameter(pContext, (vx_reference)vxCreateArray(pContext->context(), item_type, capacity))
{}

Array::~Array()
{
    vxReleaseArray((vx_array *)&m_handle);
}

vx_status Array::AddArrayItems(vx_size count, const void *ptr, vx_size stride)
{
    return vxAddArrayItems((vx_array)handle(), count, ptr, stride);
}

vx_status Array::TruncateArray(vx_size new_num_items)
{
    return vxTruncateArray((vx_array)handle(), new_num_items);
}

vx_status Array::AccessArrayRange(vx_size start, vx_size end, vx_size *stride, void **ptr, vx_enum usage)
{
    return vxAccessArrayRange((vx_array)handle(), start, end, stride, ptr, usage);
}

vx_status Array::CommitArrayRange(vx_array arrref, vx_size start, vx_size end, void *ptr)
{
    return vxCommitArrayRange((vx_array)handle(), start, end, ptr);
}

vx_size Array::numItems()
{
    vx_size num_items;
    vxQueryArray((vx_array)handle(), VX_ARRAY_NUMITEMS, &num_items, sizeof(num_items));
    return num_items;
}

vx_enum Array::itemType()
{
    vx_enum item_type;
    vxQueryArray((vx_array)handle(), VX_ARRAY_ITEMTYPE, &item_type, sizeof(item_type));
    return item_type;
}

vx_size Array::capacity()
{
    vx_size cap;
    vxQueryArray((vx_array)handle(), VX_ARRAY_CAPACITY, &cap, sizeof(cap));
    return cap;
}

vx_size Array::itemSize()
{
    vx_size item_size;
    vxQueryArray((vx_array)handle(), VX_ARRAY_ITEMSIZE, &item_size, sizeof(item_size));
    return item_size;
}
