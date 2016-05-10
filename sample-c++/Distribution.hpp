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

#ifndef _VX_DISTRIBUTION_HPP_
#define _VX_DISTRIBUTION_HPP_

namespace OpenVX {

    class VX_CLASS Distribution : public Parameter {

        friend class Context;

    protected:
        Distribution(Context* pContext, vx_size numBins, vx_size offset, vx_size range);  // vxCreateDistribution

    public:
        ~Distribution();  // vxReleaseDistribution

#ifdef VX_1_0_1_NAMING_COMPATIBILITY
        vx_status Access(void** data, vx_enum usage);  // vxAccessDistribution
        vx_status Commit(void* data);   // vxCommitDistribution
#endif
        vx_status Read(void* data); // vxCopyDistribution
        vx_status Write(void* data); // vxCopyDistribution
        vx_status Map(void** data, vx_map_id* map_id, vx_enum usage); // vxMapDistribution
        vx_status Unmap(vx_map_id map_id); // vxUnmapDistribution

        // vxQueryDistribution
        vx_size   dimensions();
        vx_uint32 range();
        vx_uint32 bins();
        vx_uint32 window();
        vx_uint32 offset();
        vx_size   size();

    };

};

#endif

