/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
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

#include "Distribution.hpp"

using namespace OpenVX;

Distribution::Distribution(Context* pContext, vx_size numBins, vx_size offset, vx_size range) :
              Parameter(pContext, (vx_reference)vxCreateDistribution(pContext->context(), numBins, offset, range))
{}

Distribution::~Distribution()
{
    vxReleaseDistribution((vx_distribution *)&m_handle);
}

#ifdef VX_1_0_1_NAMING_COMPATIBILITY
vx_status Distribution::Access(void** data, vx_enum usage)
{
    return vxAccessDistribution((vx_distribution)handle(), data, usage);
}

vx_status Distribution::Commit(void* data)
{
    return vxCommitDistribution((vx_distribution)handle(), data);
}
#endif

vx_status Distribution::Map(void** data, vx_map_id* map_id, vx_enum usage)
{
    return vxMapDistribution((vx_distribution)handle(), map_id, data, usage, VX_MEMORY_TYPE_HOST, 0);
}

vx_status Distribution::Unmap(vx_map_id map_id)
{
    return vxUnmapDistribution((vx_distribution)handle(), map_id);
}

vx_status Distribution::Read(void* data)
{
    return vxCopyDistribution((vx_distribution)handle(), data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
}

vx_status Distribution::Write(void* data)
{
    return vxCopyDistribution((vx_distribution)handle(), data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
}

// vxQueryDistribution
vx_size Distribution::dimensions()
{
    vx_size dims = 0;
    vxQueryDistribution((vx_distribution)handle(), VX_DISTRIBUTION_DIMENSIONS, &dims, sizeof(dims));
    return dims;
}

vx_uint32 Distribution::range()
{
    vx_uint32 range = 0;
    vxQueryDistribution((vx_distribution)handle(), VX_DISTRIBUTION_RANGE, &range, sizeof(range));
    return range;
}

vx_uint32 Distribution::bins()
{
    vx_uint32 v = 0;
    vxQueryDistribution((vx_distribution)handle(), VX_DISTRIBUTION_BINS, &v, sizeof(v));
    return v;
}

vx_uint32 Distribution::window()
{
    vx_uint32 v = 0;
    vxQueryDistribution((vx_distribution)handle(), VX_DISTRIBUTION_WINDOW, &v, sizeof(v));
    return v;
}

vx_uint32 Distribution::offset()
{
    vx_uint32 v = 0;
    vxQueryDistribution((vx_distribution)handle(), VX_DISTRIBUTION_OFFSET, &v, sizeof(v));
    return v;
}

vx_size Distribution::size()
{
    vx_size size = 0;
    vxQueryDistribution((vx_distribution)handle(), VX_DISTRIBUTION_SIZE, &size, sizeof(size));
    return size;
}


