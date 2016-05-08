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

#include <vx_internal.h>

VX_API_ENTRY vx_distribution VX_API_CALL vxCreateDistribution(vx_context context, vx_size numBins, vx_int32 offset, vx_uint32 range)
{
    vx_distribution distribution = NULL;

    if (vxIsValidContext(context) == vx_true_e)
    {
        if ((numBins != 0) && (range != 0))
        {
            distribution = (vx_distribution)vxCreateReference(context, VX_TYPE_DISTRIBUTION, VX_EXTERNAL, &context->base);
            if ( vxGetStatus((vx_reference)distribution) == VX_SUCCESS &&
                 distribution->base.type == VX_TYPE_DISTRIBUTION)
            {
                distribution->memory.ndims = 2;
                distribution->memory.nptrs = 1;
                distribution->memory.strides[0][VX_DIM_C] = sizeof(vx_int32);
                distribution->memory.dims[0][VX_DIM_C] = 1;
                distribution->memory.dims[0][VX_DIM_X] = (vx_int32)numBins;
                distribution->memory.dims[0][VX_DIM_Y] = 1;
				distribution->memory.cl_type = CL_MEM_OBJECT_BUFFER;
                distribution->range_x = (vx_uint32)range;
                distribution->range_y = 1;
                distribution->offset_x = offset;
                distribution->offset_y = 0;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid parameters to distribution\n");
            vxAddLogEntry(&context->base, VX_ERROR_INVALID_PARAMETERS, "Invalid parameters to distribution\n");
            distribution = (vx_distribution)vxGetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
        }
    }

    return distribution;
}

void vxDestructDistribution(vx_reference ref)
{
    vx_distribution distribution = (vx_distribution)ref;
    vxFreeMemory(distribution->base.context, &distribution->memory);
}

vx_status vxReleaseDistributionInt(vx_distribution *distribution)
{
    return vxReleaseReferenceInt((vx_reference *)distribution, VX_TYPE_DISTRIBUTION, VX_INTERNAL, &vxDestructDistribution);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseDistribution(vx_distribution *d)
{
    return vxReleaseReferenceInt((vx_reference *)d, VX_TYPE_DISTRIBUTION, VX_EXTERNAL, &vxDestructDistribution);
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryDistribution(vx_distribution distribution, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (vxIsValidSpecificReference(&distribution->base, VX_TYPE_DISTRIBUTION) == vx_false_e)
        return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_DISTRIBUTION_DIMENSIONS:
            if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
            {
                *(vx_size*)ptr = (vx_size)(distribution->memory.ndims - 1);
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_DISTRIBUTION_RANGE:
            if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
            {
                *(vx_uint32*)ptr = (vx_uint32)(distribution->range_x);
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_DISTRIBUTION_BINS:
            if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
            {
                *(vx_size*)ptr = (vx_size)distribution->memory.dims[0][VX_DIM_X];
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_DISTRIBUTION_WINDOW:
            if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
            {
                vx_size nbins = (vx_size)distribution->memory.dims[0][VX_DIM_X];
                vx_uint32 range = (vx_uint32)(distribution->range_x);
                vx_uint32 window = range / nbins;
                if (window*nbins == range)
                    *(vx_uint32*)ptr = window;
                else
                    *(vx_uint32*)ptr = 0;
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_DISTRIBUTION_OFFSET:
            if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3))
            {
                *(vx_int32*)ptr = distribution->offset_x;
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_DISTRIBUTION_SIZE:
            if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
            {
                vx_int32 d = distribution->memory.ndims - 1;
                *(vx_size*)ptr = distribution->memory.strides[0][d] *
                                    distribution->memory.dims[0][d];
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        default:
            status = VX_ERROR_NOT_SUPPORTED;
            break;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxAccessDistribution(vx_distribution distribution, void **ptr, vx_enum usage)
{
    vx_status status = VX_FAILURE;
    if ((vxIsValidSpecificReference(&distribution->base, VX_TYPE_DISTRIBUTION) == vx_true_e) &&
        (vxAllocateMemory(distribution->base.context, &distribution->memory) == vx_true_e))
    {
        if (ptr != NULL)
        {
            vxSemWait(&distribution->base.lock);
            {
                vx_size size = vxComputeMemorySize(&distribution->memory, 0);
                vxPrintMemory(&distribution->memory);
                if (*ptr == NULL)
                {
                    *ptr = distribution->memory.ptrs[0];
                }
                else if (*ptr != NULL)
                {
                    memcpy(*ptr, distribution->memory.ptrs[0], size);
                }
            }
            vxSemPost(&distribution->base.lock);
            vxReadFromReference(&distribution->base);
        }
        vxIncrementReference(&distribution->base, VX_EXTERNAL);
        status = VX_SUCCESS;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Not a valid object!\n");
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxCommitDistribution(vx_distribution distribution, const void *ptr)
{
    vx_status status = VX_FAILURE;
    if ((vxIsValidSpecificReference(&distribution->base, VX_TYPE_DISTRIBUTION) == vx_true_e) &&
        (vxAllocateMemory(distribution->base.context, &distribution->memory) == vx_true_e))
    {
        if (ptr != NULL)
        {
            vxSemWait(&distribution->base.lock);
            {
                if (ptr != distribution->memory.ptrs[0])
                {
                    vx_size size = vxComputeMemorySize(&distribution->memory, 0);
                    memcpy(distribution->memory.ptrs[0], ptr, size);
                    VX_PRINT(VX_ZONE_INFO, "Copied distribution from %p to %p for "VX_FMT_SIZE" bytes\n", ptr, distribution->memory.ptrs[0], size);
                }
            }
            vxSemPost(&distribution->base.lock);
            vxWroteToReference(&distribution->base);
        }
        vxDecrementReference(&distribution->base, VX_EXTERNAL);
        status = VX_SUCCESS;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Not a valid object!\n");
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxCopyDistribution(vx_distribution distribution, void *user_ptr, vx_enum usage, vx_enum mem_type)
{
    vx_status status = VX_FAILURE;
    vx_size size = 0;

    /* bad references */
    if ((vxIsValidSpecificReference(&distribution->base, VX_TYPE_DISTRIBUTION) != vx_true_e) ||
        (vxAllocateMemory(distribution->base.context, &distribution->memory) != vx_true_e))
    {
        status = VX_ERROR_INVALID_REFERENCE;
        VX_PRINT(VX_ZONE_ERROR, "Not a valid distribution object!\n");
        return status;
    }

    /* bad parameters */
    if (((usage != VX_READ_ONLY) && (usage != VX_WRITE_ONLY)) ||
        (user_ptr == NULL) || (mem_type != VX_MEMORY_TYPE_HOST))
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "Invalid parameters to copy distribution\n");
        return status;
    }

    /* copy data */
    size = vxComputeMemorySize(&distribution->memory, 0);
    vxPrintMemory(&distribution->memory);

    switch (usage)
    {
    case VX_READ_ONLY:
        if (vxSemWait(&distribution->base.lock) == vx_true_e)
        {
            memcpy(user_ptr, distribution->memory.ptrs[0], size);
            vxSemPost(&distribution->base.lock);

            vxReadFromReference(&distribution->base);
            status = VX_SUCCESS;
        }
        break;
    case VX_WRITE_ONLY:
        if (vxSemWait(&distribution->base.lock) == vx_true_e)
        {
            memcpy(distribution->memory.ptrs[0], user_ptr, size);
            vxSemPost(&distribution->base.lock);

            vxWroteToReference(&distribution->base);
            status = VX_SUCCESS;
        }
        break;
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxMapDistribution(vx_distribution distribution, vx_map_id *map_id, void **ptr, vx_enum usage, vx_enum mem_type, vx_bitfield flags)
{
    vx_status status = VX_FAILURE;
    vx_size size = 0;

    /* bad references */
    if ((vxIsValidSpecificReference(&distribution->base, VX_TYPE_DISTRIBUTION) != vx_true_e) ||
        (vxAllocateMemory(distribution->base.context, &distribution->memory) != vx_true_e))
    {
        status = VX_ERROR_INVALID_REFERENCE;
        VX_PRINT(VX_ZONE_ERROR, "Not a valid distribution object!\n");
        return status;
    }

    /* bad parameters */
    if (((usage != VX_READ_ONLY) && (usage != VX_READ_AND_WRITE) && (usage != VX_WRITE_ONLY)) ||
        (mem_type != VX_MEMORY_TYPE_HOST))
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "Invalid parameters to map distribution\n");
        return status;
    }

    /* map data */
    size = vxComputeMemorySize(&distribution->memory, 0);
    vxPrintMemory(&distribution->memory);

    if (vxMemoryMap(distribution->base.context, (vx_reference)distribution, size, usage, mem_type, flags, NULL, ptr, map_id) == vx_true_e)
    {
        switch (usage)
        {
        case VX_READ_ONLY:
        case VX_READ_AND_WRITE:
            if (vxSemWait(&distribution->base.lock) == vx_true_e)
            {
                memcpy(*ptr, distribution->memory.ptrs[0], size);
                vxSemPost(&distribution->base.lock);

                vxReadFromReference(&distribution->base);
                status = VX_SUCCESS;
            }
            break;
        case VX_WRITE_ONLY:
            status = VX_SUCCESS;
            break;
        }

        if (status == VX_SUCCESS)
            vxIncrementReference(&distribution->base, VX_EXTERNAL);
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxUnmapDistribution(vx_distribution distribution, vx_map_id map_id)
{
    vx_status status = VX_FAILURE;
    vx_size size = 0;

    /* bad references */
    if ((vxIsValidSpecificReference(&distribution->base, VX_TYPE_DISTRIBUTION) != vx_true_e) ||
        (vxAllocateMemory(distribution->base.context, &distribution->memory) != vx_true_e))
    {
        status = VX_ERROR_INVALID_REFERENCE;
        VX_PRINT(VX_ZONE_ERROR, "Not a valid distribution object!\n");
        return status;
    }

    /* bad parameters */
    if (vxFindMemoryMap(distribution->base.context, (vx_reference)distribution, map_id) != vx_true_e)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "Invalid parameters to unmap distribution\n");
        return status;
    }

    /* unmap data */
    size = vxComputeMemorySize(&distribution->memory, 0);
    vxPrintMemory(&distribution->memory);

    {
        vx_uint32 id = (vx_uint32)map_id;
        vx_memory_map_t* map = &distribution->base.context->memory_maps[id];

        switch (map->usage)
        {
        case VX_READ_ONLY:
            status = VX_SUCCESS;
            break;
        case VX_READ_AND_WRITE:
        case VX_WRITE_ONLY:
            if (vxSemWait(&distribution->base.lock) == vx_true_e)
            {
                memcpy(distribution->memory.ptrs[0], map->ptr, size);
                vxSemPost(&distribution->base.lock);

                vxWroteToReference(&distribution->base);
                status = VX_SUCCESS;
            }
            break;
        }

        vxMemoryUnmap(distribution->base.context, map_id);

        /* regardless of the current status, if we're here, so previous call to vxMapDistribution()
         * was successful and thus ref was locked once by a call to vxIncrementReference() */
        vxDecrementReference(&distribution->base, VX_EXTERNAL);
    }

    return status;
}

