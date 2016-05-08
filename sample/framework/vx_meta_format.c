/*
 * Copyright (c) 2016-2016 The Khronos Group Inc.
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

#include <vx_internal.h>

void vxReleaseMetaFormat(vx_meta_format *pmeta)
{
    vxReleaseReferenceInt((vx_reference *)pmeta, VX_TYPE_META_FORMAT, VX_EXTERNAL, NULL);
}

vx_meta_format vxCreateMetaFormat(vx_context context)
{
    vx_meta_format meta = NULL;

    if (vxIsValidContext(context) == vx_true_e)
    {
        meta = (vx_meta_format)vxCreateReference(context, VX_TYPE_META_FORMAT, VX_EXTERNAL, &context->base);
        if (vxGetStatus((vx_reference)meta) == VX_SUCCESS)
        {
            meta->size = sizeof(vx_meta_format_t);
            meta->type = VX_TYPE_INVALID;
        }
    }

    return meta;
}

/******************************************************************************/
// PUBLIC
/******************************************************************************/

VX_API_ENTRY vx_status VX_API_CALL vxSetMetaFormatAttribute(vx_meta_format meta, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (vxIsValidSpecificReference(&meta->base, VX_TYPE_META_FORMAT) == vx_false_e)
        return VX_ERROR_INVALID_REFERENCE;

    if (VX_TYPE(attribute) != meta->type) {
        return VX_ERROR_INVALID_TYPE;
    }

    switch(attribute) {
        case VX_IMAGE_FORMAT:
            if (VX_CHECK_PARAM(ptr, size, vx_df_image, 0x3)) {
                meta->dim.image.format = *(vx_df_image *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_IMAGE_HEIGHT:
            if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3)) {
                meta->dim.image.height = *(vx_uint32 *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_IMAGE_WIDTH:
            if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3)) {
                meta->dim.image.width = *(vx_uint32 *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        /**********************************************************************/
        case VX_ARRAY_CAPACITY:
            if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3)) {
                meta->dim.array.capacity = *(vx_size *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_ARRAY_ITEMTYPE:
            if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3)) {
                meta->dim.array.item_type = *(vx_enum *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        /**********************************************************************/
        case VX_PYRAMID_FORMAT:
            if (VX_CHECK_PARAM(ptr, size, vx_df_image, 0x3)) {
                meta->dim.pyramid.format = *(vx_df_image *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_PYRAMID_HEIGHT:
            if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3)) {
                meta->dim.pyramid.height = *(vx_uint32 *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_PYRAMID_WIDTH:
            if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3)) {
                meta->dim.pyramid.width = *(vx_uint32 *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_PYRAMID_LEVELS:
            if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3)) {
                meta->dim.pyramid.levels = *(vx_size *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_PYRAMID_SCALE:
            if (VX_CHECK_PARAM(ptr, size, vx_float32, 0x3)) {
                meta->dim.pyramid.scale = *(vx_float32 *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        /**********************************************************************/
        case VX_SCALAR_TYPE:
            if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3)) {
                meta->dim.scalar.type = *(vx_enum *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        /**********************************************************************/
        case VX_MATRIX_TYPE:
            if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3)) {
                meta->dim.matrix.type = *(vx_enum *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_MATRIX_ROWS:
            if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3)) {
                meta->dim.matrix.rows = *(vx_size *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_MATRIX_COLUMNS:
            if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3)) {
                meta->dim.matrix.cols = *(vx_size *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        /**********************************************************************/
        case VX_DISTRIBUTION_BINS:
            if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3)) {
                meta->dim.distribution.bins = *(vx_size *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_DISTRIBUTION_RANGE:
            if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3)) {
                meta->dim.distribution.range = *(vx_uint32 *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_DISTRIBUTION_OFFSET:
            if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3)) {
                meta->dim.distribution.offset = *(vx_int32 *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        /**********************************************************************/
        case VX_REMAP_SOURCE_WIDTH:
            if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3)) {
                meta->dim.remap.src_width = *(vx_uint32 *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_REMAP_SOURCE_HEIGHT:
            if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3)) {
                meta->dim.remap.src_height = *(vx_uint32 *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_REMAP_DESTINATION_WIDTH:
            if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3)) {
                meta->dim.remap.dst_width = *(vx_uint32 *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_REMAP_DESTINATION_HEIGHT:
            if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3)) {
                meta->dim.remap.dst_height = *(vx_uint32 *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        /**********************************************************************/
        case VX_LUT_TYPE:
            if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3)) {
                meta->dim.lut.type = *(vx_enum *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_LUT_COUNT:
            if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3)) {
                meta->dim.lut.count = *(vx_size *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        /**********************************************************************/
        case VX_THRESHOLD_TYPE:
            if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3)) {
                meta->dim.threshold.type = *(vx_enum *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        default:
            status = VX_ERROR_NOT_SUPPORTED;
            break;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetMetaFormatFromReference(vx_meta_format meta, vx_reference examplar)
{
    vx_status status = VX_SUCCESS;

    if (vxIsValidSpecificReference(&meta->base, VX_TYPE_META_FORMAT) == vx_false_e)
        return VX_ERROR_INVALID_REFERENCE;

    if (vxIsValidReference(examplar) == vx_false_e)
        return VX_ERROR_INVALID_REFERENCE;

    switch (examplar->type)
    {
    case VX_TYPE_IMAGE:
    {
        vx_image image = (vx_image)examplar;
        meta->type = VX_TYPE_IMAGE;
        meta->dim.image.width = image->width;
        meta->dim.image.height = image->height;
        meta->dim.image.format = image->format;
        break;
    }
    case VX_TYPE_ARRAY:
    {
        vx_array array = (vx_array)examplar;
        meta->type = VX_TYPE_ARRAY;
        meta->dim.array.item_type = array->item_type;
        meta->dim.array.capacity = array->capacity;
        break;
    }
    case VX_TYPE_PYRAMID:
    {
        vx_pyramid pyramid = (vx_pyramid)examplar;
        meta->type = VX_TYPE_PYRAMID;
        meta->dim.pyramid.width = pyramid->width;
        meta->dim.pyramid.height = pyramid->height;
        meta->dim.pyramid.format = pyramid->format;
        meta->dim.pyramid.levels = pyramid->numLevels;
        meta->dim.pyramid.scale = pyramid->scale;
        break;
    }
    case VX_TYPE_SCALAR:
    {
        vx_scalar scalar = (vx_scalar)examplar;
        meta->type = VX_TYPE_SCALAR;
        meta->dim.scalar.type = scalar->data_type;
        break;
    }
    case VX_TYPE_MATRIX:
    {
        vx_matrix matrix = (vx_matrix)examplar;
        meta->type = VX_TYPE_MATRIX;
        meta->dim.matrix.type = matrix->data_type;
        meta->dim.matrix.cols = matrix->columns;
        meta->dim.matrix.rows = matrix->rows;
        break;
    }
    case VX_TYPE_DISTRIBUTION:
    {
        vx_distribution distribution = (vx_distribution)examplar;
        meta->type = VX_TYPE_DISTRIBUTION;
        meta->dim.distribution.bins = distribution->memory.dims[0][VX_DIM_X];
        meta->dim.distribution.offset = distribution->offset_x;
        meta->dim.distribution.range = distribution->range_x;
        break;
    }
    case VX_TYPE_REMAP:
    {
        vx_remap remap = (vx_remap)examplar;
        meta->type = VX_TYPE_REMAP;
        meta->dim.remap.src_width = remap->src_width;
        meta->dim.remap.src_height = remap->src_height;
        meta->dim.remap.dst_width = remap->dst_width;
        meta->dim.remap.dst_height = remap->dst_height;
        break;
    }
    case VX_TYPE_LUT:
    {
        vx_lut_t *lut = (vx_lut_t *)examplar;
        meta->type = VX_TYPE_LUT;
        meta->dim.lut.type = lut->item_type;
        meta->dim.lut.count = lut->num_items;
        break;
    }
    case VX_TYPE_THRESHOLD:
    {
        vx_threshold threshold = (vx_threshold)examplar;
        meta->type = VX_TYPE_THRESHOLD;
        meta->dim.threshold.type = threshold->thresh_type;
        break;
    }
    default:
        status = VX_ERROR_INVALID_REFERENCE;
        break;
    }

    return status;
}
