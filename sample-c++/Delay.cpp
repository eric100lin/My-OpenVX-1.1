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
#include "Delay.hpp"

namespace OpenVX {

// constructors and [] operators (vxGetXXFromDelay) need to be specialized

template <> Delay<Image>  :: Delay(Context* pContext, vx_uint32 width, vx_uint32 height, vx_df_image format, vx_size num) :
                             /*! \todo need exemplar object here (width, height, format,) */
                             Parameter(pContext, (vx_reference)vxCreateDelay(pContext->context(), 0, num))
{}

template <> Image* Delay<Image>::operator[](vx_uint32 index)
{
    vx_image img = (vx_image)vxGetReferenceFromDelay((vx_delay)handle(), index);
    return (img ? Image::wrap(context(), img) : NULL);
}

// un-specialized functions

template <class T> Delay<T>::~Delay()
{
    vxReleaseDelay((vx_delay *)&m_handle);
}

template <class T> vx_status Delay<T>::associate(vx_int32 delay_index, Node* pNode, vx_uint32 param_index, vx_enum param_direction)
{
    // BUGBUG need a vx_ref-free implementation of this
    return VX_ERROR_NOT_SUPPORTED;
    //vxAssociateDelayWithNode((vx_delay)handle(), delay_index, pNode->handle(), param_index, param_direction);
}

template <class T> vx_status Delay<T>::dissociate(vx_int32 delay_index, Node* pNode, vx_uint32 param_index)
{
    // BUGBUG need a vx_ref-free implementation of this
    return VX_ERROR_NOT_SUPPORTED;
    //vxDissociateDelayFromNode((vx_delay)handle(), delay_index, pNode->handle(), param_index);
}

template <class T> vx_status Delay<T>::age()
{
    return vxAgeDelay((vx_delay)handle());
}

template <class T> vx_enum Delay<T>::type()
{
    vx_enum v;
    vxQueryDelay((vx_delay)handle(), VX_DELAY_TYPE, &v, sizeof(v));
    return v;
}

template <class T> vx_size Delay<T>::count()
{
    vx_size v;
    vxQueryDelay((vx_delay)handle(), VX_DELAY_SLOTS, &v, sizeof(v));
    return v;
}

}

