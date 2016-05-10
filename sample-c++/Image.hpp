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

#ifndef _VX_IMAGE_HPP_
#define _VX_IMAGE_HPP_

namespace OpenVX {

    class VX_CLASS Image : public Parameter {

        friend class Context;
        friend class Delay<Image>;
        friend class Pyramid;

    protected:
        // wrap a vx_image with an Image
        static Image *wrap(Context* pContext, vx_image ref)
        {
            Reference *r = new Reference(pContext, (vx_reference)ref);
            return dynamic_cast<Image *>(r);
        }

        /*
        Image(Context* pContext);                                                                                   // vxCreateVirtualImage
        Image(Context* pContext, vx_df_image color);                                                                  // vxCreateVirtualImageWithFormat
        */
        Image(Context* pContext, vx_uint32 width, vx_uint32 height, vx_df_image color);                               // vxCreateImage
        Image(Context* pContext, vx_df_image color, vx_imagepatch_addressing_t addrs[], void* ptrs[], vx_enum type);  // vxCreateImageFromHandle
        Image(Context* pContext, vx_uint32 width, vx_uint32 height, vx_df_image color, vx_pixel_value_t* value);                  // vxCreateUniformImage
        Image(Image* pImg, vx_rectangle_t *rect);                                                                      // vxCreateImageFromROI

    public:
        ~Image();

        vx_status    AccessImagePatch          (vx_rectangle_t *rect, vx_uint32 p, vx_imagepatch_addressing_t *addr, void **ptr, vx_enum usage);    // vxAccessImagePatch
        vx_status    CommitImagePatch          (vx_rectangle_t *rect, vx_uint32 p, vx_imagepatch_addressing_t *addr, void *ptr);     // vxCommitImagePatch
        vx_size      ComputeImagePatchSize     (vx_rectangle_t *rect, vx_uint32 p);                                                  // vxComputeImagePatchSize

        void*        FormatImagePatchAddress1d (void *ptr, vx_uint32 index, vx_imagepatch_addressing_t* addr);                  // vxFormatImagePatchAddress1d
        void*        FormatImagePatchAddress2d (void *ptr, vx_uint32 x, vx_uint32 y, vx_imagepatch_addressing_t* addr);         // vxFormatImagePatchAddress2d
        vx_rectangle_t GetValidRegionImage       (Image* pImage);                                                                 // vxGetValidRegionImage

        // various incarnations of vxQueryImage
        vx_uint32   width();
        vx_uint32   height();
        vx_df_image   format();
        vx_uint32   planes();
        vx_enum     space();
        vx_enum     range();
    };

}
#endif

