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

#ifndef _VX_ARRAY_HPP_
#define _VX_ARRAY_HPP_

namespace OpenVX {

    class VX_CLASS Array: public Parameter {

        friend class Context;
        friend class Delay<Array>;

    protected:
        static Array *wrap(Context* pContext, vx_array ref)
        {
            Reference *r = new Reference(pContext, (vx_reference)ref);
            return dynamic_cast<Array *>(r);
        }

        //Array(Graph *pGraph, vx_enum item_type, vx_size capacity);                                 // vxCreateVirtualArray
        Array(Context* pContext, vx_enum item_type, vx_size capacity);                               // vxCreateArray

    public:
        ~Array();                                                               // vxReleaseArray

        vx_status AddArrayItems(vx_size count, const void *ptr, vx_size stride=0);    // vxAddArrayItems
        vx_status TruncateArray(vx_size new_num_items);                         // vxTruncateArray
        vx_status AccessArrayRange(vx_size start, vx_size end, vx_size *stride, void **ptr, vx_enum usage); // vxAccessArrayRange
        vx_status CommitArrayRange(vx_array arrref, vx_size start, vx_size end, void *ptr); // vxCommitArrayRange

        // various incarnations of vxQueryArray
        vx_size numItems();
        vx_enum itemType();
        vx_size capacity();
        vx_size itemSize();
    };

};

#endif
