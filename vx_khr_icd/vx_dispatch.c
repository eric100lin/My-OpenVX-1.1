/*
 * Copyright (c) 2016 The Khronos Group Inc.
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

#define _CRT_SECURE_NO_WARNINGS
#include "vx_dispatch.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#if _WIN32
#include <Windows.h>
#define OpenSharedLibrary(libraryPath)      LoadLibraryA(libraryPath)
#define GetFunctionAddress(handle,funcName) GetProcAddress(handle,funcName)
#else
#include <dlfcn.h>
#define OpenSharedLibrary(libraryPath)      dlopen(libraryPath, RTLD_NOW|RTLD_LOCAL)
#define GetFunctionAddress(handle,funcName) dlsym(handle,funcName)
#endif

#define CHECK_OBJECT(obj,retValue)                 if(!obj) return retValue
#define CHECK_OBJECT_VOID(obj)                     if(!obj) return
#define CHECK_OBJECT_PTR(objPtr,retValue)          if(!objPtr || !*objPtr) return retValue
#define CHECK_DISPATCH(platform,funcName,retValue) if(!platform || !platform->funcName) return retValue
#define CHECK_DISPATCH_VOID(platform,funcName)     if(!platform || !platform->funcName) return

static vx_status VX_API_ENTRY vxLoadDispatchTable(vx_platform platform);

vx_context VX_API_CALL vxCreateContextFromPlatform(vx_platform platform)
{
    if(platform && !platform->handle_openvx) {
        vx_status status = vxLoadDispatchTable(platform);
        if(status != VX_SUCCESS)
            return NULL;
    }
    CHECK_DISPATCH(platform, vxCreateContextFromPlatform, NULL);
    vx_context context = platform->vxCreateContextFromPlatform(platform);
    if(!platform->attributes_valid && vxGetStatus((vx_reference)context) == VX_SUCCESS) {
        platform->attributes_valid = vx_true_e;
        vxQueryContext(context, VX_CONTEXT_VENDOR_ID, &platform->vendor_id, sizeof(platform->vendor_id));
        vxQueryContext(context, VX_CONTEXT_VERSION, &platform->version, sizeof(platform->version));
        vxQueryContext(context, VX_CONTEXT_EXTENSIONS_SIZE, &platform->extensions_size, sizeof(platform->extensions_size));
        if(platform->extensions_size > 0) {
            platform->extensions = (vx_char *)calloc(1, platform->extensions_size + 1);
            if(!platform->extensions) {
                platform->extensions_size = 0;
            }
            else {
                vxQueryContext(context, VX_CONTEXT_EXTENSIONS, platform->extensions, platform->extensions_size);
            }
        }
        if(platform->version == VX_VERSION_1_0) {
            platform->vxHint101 = (TYPE_vxHint101 *)platform->vxHint;
            platform->vxHint = NULL;
        }
    }
    return context;
}

vx_context VX_API_CALL vxCreateContext()
{
    vx_context context = NULL;
    vx_platform platform = NULL;
    vx_status status = vxIcdGetPlatforms(1, &platform, NULL);
    if(status == VX_SUCCESS)
        context = vxCreateContextFromPlatform(platform);
    return context;
}

vx_status VX_API_CALL vxReleaseContext(vx_context *context)
{
    CHECK_OBJECT_PTR(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) *context;
    CHECK_DISPATCH(platform, vxReleaseContext, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxReleaseContext(context);
}

vx_context VX_API_CALL vxGetContext(vx_reference reference)
{
    CHECK_OBJECT(reference, NULL);
    vx_platform platform = *(vx_platform *) reference;
    CHECK_DISPATCH(platform, vxGetContext, NULL);
    return platform->vxGetContext(reference);
}

vx_status VX_API_CALL vxQueryContext(vx_context context, vx_enum attribute, void *ptr, vx_size size)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxQueryContext, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxQueryContext(context, attribute, ptr, size);
}

vx_status VX_API_CALL vxSetContextAttribute(vx_context context, vx_enum attribute, const void *ptr, vx_size size)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxSetContextAttribute, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxSetContextAttribute(context, attribute, ptr, size);
}

vx_status VX_API_CALL vxHint(vx_reference reference, vx_enum hint, const void* data, vx_size data_size)
{
    CHECK_OBJECT(reference, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) reference;
    CHECK_OBJECT(platform, VX_ERROR_NOT_IMPLEMENTED);
    if(platform->vxHint)
        return platform->vxHint(reference, hint, data, data_size);
    else if(platform->vxHint101)
        return platform->vxHint101(reference, hint);
    else
        return VX_ERROR_NOT_IMPLEMENTED;
}

vx_status VX_API_CALL vxDirective(vx_reference reference, vx_enum directive)
{
    CHECK_OBJECT(reference, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) reference;
    CHECK_DISPATCH(platform, vxDirective, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxDirective(reference, directive);
}

vx_status VX_API_CALL vxGetStatus(vx_reference reference)
{
    CHECK_OBJECT(reference, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) reference;
    CHECK_DISPATCH(platform, vxGetStatus, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxGetStatus(reference);
}

vx_enum VX_API_CALL vxRegisterUserStruct(vx_context context, vx_size size)
{
    CHECK_OBJECT(context, VX_TYPE_INVALID);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxRegisterUserStruct, VX_TYPE_INVALID);
    return platform->vxRegisterUserStruct(context, size);
}

vx_status VX_API_CALL vxAllocateUserKernelId(vx_context context, vx_enum * pKernelEnumId)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxAllocateUserKernelId, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxAllocateUserKernelId(context, pKernelEnumId);
}

vx_status VX_API_CALL vxAllocateUserKernelLibraryId(vx_context context, vx_enum * pLibraryId)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxAllocateUserKernelLibraryId, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxAllocateUserKernelLibraryId(context, pLibraryId);
}

vx_status VX_API_CALL vxSetImmediateModeTarget(vx_context context, vx_enum target_enum, const char* target_string)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxSetImmediateModeTarget, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxSetImmediateModeTarget(context, target_enum, target_string);
}

vx_image VX_API_CALL vxCreateImage(vx_context context, vx_uint32 width, vx_uint32 height, vx_df_image color)
{
    CHECK_OBJECT(context, NULL);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxCreateImage, NULL);
    return platform->vxCreateImage(context, width, height, color);
}

vx_image VX_API_CALL vxCreateImageFromROI(vx_image img, const vx_rectangle_t *rect)
{
    CHECK_OBJECT(img, NULL);
    vx_platform platform = *(vx_platform *) img;
    CHECK_DISPATCH(platform, vxCreateImageFromROI, NULL);
    return platform->vxCreateImageFromROI(img, rect);
}

vx_image VX_API_CALL vxCreateUniformImage(vx_context context, vx_uint32 width, vx_uint32 height, vx_df_image color, const vx_pixel_value_t *value)
{
    CHECK_OBJECT(context, NULL);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxCreateUniformImage, NULL);
    return platform->vxCreateUniformImage(context, width, height, color, value);
}

vx_image VX_API_CALL vxCreateVirtualImage(vx_graph graph, vx_uint32 width, vx_uint32 height, vx_df_image color)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxCreateVirtualImage, NULL);
    return platform->vxCreateVirtualImage(graph, width, height, color);
}

vx_image VX_API_CALL vxCreateImageFromHandle(vx_context context, vx_df_image color, const vx_imagepatch_addressing_t addrs[], void *const ptrs[], vx_enum memory_type)
{
    CHECK_OBJECT(context, NULL);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxCreateImageFromHandle, NULL);
    return platform->vxCreateImageFromHandle(context, color, addrs, ptrs, memory_type);
}

vx_status VX_API_CALL vxSwapImageHandle(vx_image image, void* const new_ptrs[], void* prev_ptrs[], vx_size num_planes)
{
    CHECK_OBJECT(image, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) image;
    CHECK_DISPATCH(platform, vxSwapImageHandle, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxSwapImageHandle(image, new_ptrs, prev_ptrs, num_planes);
}

vx_status VX_API_CALL vxQueryImage(vx_image image, vx_enum attribute, void *ptr, vx_size size)
{
    CHECK_OBJECT(image, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) image;
    CHECK_DISPATCH(platform, vxQueryImage, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxQueryImage(image, attribute, ptr, size);
}

vx_status VX_API_CALL vxSetImageAttribute(vx_image image, vx_enum attribute, const void *ptr, vx_size size)
{
    CHECK_OBJECT(image, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) image;
    CHECK_DISPATCH(platform, vxSetImageAttribute, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxSetImageAttribute(image, attribute, ptr, size);
}

vx_status VX_API_CALL vxReleaseImage(vx_image *image)
{
    CHECK_OBJECT_PTR(image, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) *image;
    CHECK_DISPATCH(platform, vxReleaseImage, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxReleaseImage(image);
}

vx_size VX_API_CALL vxComputeImagePatchSize(vx_image image, const vx_rectangle_t *rect, vx_uint32 plane_index)
{
    CHECK_OBJECT(image, 0);
    vx_platform platform = *(vx_platform *) image;
    CHECK_DISPATCH(platform, vxComputeImagePatchSize, 0);
    return platform->vxComputeImagePatchSize(image, rect, plane_index);
}

vx_status VX_API_CALL vxAccessImagePatch(vx_image image, const vx_rectangle_t *rect, vx_uint32 plane_index, vx_imagepatch_addressing_t *addr, void **ptr, vx_enum usage)
{
    CHECK_OBJECT(image, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) image;
    CHECK_DISPATCH(platform, vxAccessImagePatch, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxAccessImagePatch(image, rect, plane_index, addr, ptr, usage);
}

vx_status VX_API_CALL vxCommitImagePatch(vx_image image, const vx_rectangle_t *rect, vx_uint32 plane_index, const vx_imagepatch_addressing_t *addr, const void *ptr)
{
    CHECK_OBJECT(image, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) image;
    CHECK_DISPATCH(platform, vxCommitImagePatch, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxCommitImagePatch(image, rect, plane_index, addr, ptr);
}

vx_status VX_API_CALL vxGetValidRegionImage(vx_image image, vx_rectangle_t *rect)
{
    CHECK_OBJECT(image, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) image;
    CHECK_DISPATCH(platform, vxGetValidRegionImage, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxGetValidRegionImage(image, rect);
}

vx_status VX_API_CALL vxCopyImagePatch(vx_image image, const vx_rectangle_t *image_rect, vx_uint32 image_plane_index, const vx_imagepatch_addressing_t *user_addr, void * user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    CHECK_OBJECT(image, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) image;
    CHECK_DISPATCH(platform, vxCopyImagePatch, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxCopyImagePatch(image, image_rect, image_plane_index, user_addr, user_ptr, usage, user_mem_type);
}

vx_status VX_API_CALL vxMapImagePatch(vx_image image, const vx_rectangle_t *rect, vx_uint32 plane_index, vx_map_id *map_id, vx_imagepatch_addressing_t *addr, void **ptr, vx_enum usage, vx_enum mem_type, vx_uint32 flags)
{
    CHECK_OBJECT(image, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) image;
    CHECK_DISPATCH(platform, vxMapImagePatch, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxMapImagePatch(image, rect, plane_index, map_id, addr, ptr, usage, mem_type, flags);
}

vx_status VX_API_CALL vxUnmapImagePatch(vx_image image, vx_map_id map_id)
{
    CHECK_OBJECT(image, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) image;
    CHECK_DISPATCH(platform, vxUnmapImagePatch, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxUnmapImagePatch(image, map_id);
}

vx_image VX_API_CALL vxCreateImageFromChannel(vx_image img, vx_enum channel)
{
    CHECK_OBJECT(img, NULL);
    vx_platform platform = *(vx_platform *) img;
    CHECK_DISPATCH(platform, vxCreateImageFromChannel, NULL);
    return platform->vxCreateImageFromChannel(img, channel);
}

vx_status VX_API_CALL vxSetImageValidRectangle(vx_image image, const vx_rectangle_t *rect)
{
    CHECK_OBJECT(image, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) image;
    CHECK_DISPATCH(platform, vxSetImageValidRectangle, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxSetImageValidRectangle(image, rect);
}

vx_status VX_API_CALL vxLoadKernels(vx_context context, const vx_char *module)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxLoadKernels, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxLoadKernels(context, module);
}

vx_status VX_API_CALL vxUnloadKernels(vx_context context, const vx_char *module)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxUnloadKernels, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxUnloadKernels(context, module);
}

vx_kernel VX_API_CALL vxGetKernelByName(vx_context context, const vx_char *name)
{
    CHECK_OBJECT(context, NULL);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxGetKernelByName, NULL);
    return platform->vxGetKernelByName(context, name);
}

vx_kernel VX_API_CALL vxGetKernelByEnum(vx_context context, vx_enum kernel)
{
    CHECK_OBJECT(context, NULL);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxGetKernelByEnum, NULL);
    return platform->vxGetKernelByEnum(context, kernel);
}

vx_status VX_API_CALL vxQueryKernel(vx_kernel kernel, vx_enum attribute, void *ptr, vx_size size)
{
    CHECK_OBJECT(kernel, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) kernel;
    CHECK_DISPATCH(platform, vxQueryKernel, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxQueryKernel(kernel, attribute, ptr, size);
}

vx_status VX_API_CALL vxReleaseKernel(vx_kernel *kernel)
{
    CHECK_OBJECT_PTR(kernel, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) *kernel;
    CHECK_DISPATCH(platform, vxReleaseKernel, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxReleaseKernel(kernel);
}

vx_kernel VX_API_CALL vxAddUserKernel(vx_context context, const vx_char name[VX_MAX_KERNEL_NAME], vx_enum enumeration, vx_kernel_f func_ptr, vx_uint32 numParams, vx_kernel_validate_f validate, vx_kernel_initialize_f init, vx_kernel_deinitialize_f deinit)
{
    CHECK_OBJECT(context, NULL);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxAddUserKernel, NULL);
    return platform->vxAddUserKernel(context, name, enumeration, func_ptr, numParams, validate, init, deinit);
}

vx_status VX_API_CALL vxFinalizeKernel(vx_kernel kernel)
{
    CHECK_OBJECT(kernel, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) kernel;
    CHECK_DISPATCH(platform, vxFinalizeKernel, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxFinalizeKernel(kernel);
}

vx_status VX_API_CALL vxAddParameterToKernel(vx_kernel kernel, vx_uint32 index, vx_enum dir, vx_enum data_type, vx_enum state)
{
    CHECK_OBJECT(kernel, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) kernel;
    CHECK_DISPATCH(platform, vxAddParameterToKernel, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxAddParameterToKernel(kernel, index, dir, data_type, state);
}

vx_status VX_API_CALL vxRemoveKernel(vx_kernel kernel)
{
    CHECK_OBJECT(kernel, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) kernel;
    CHECK_DISPATCH(platform, vxRemoveKernel, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxRemoveKernel(kernel);
}

vx_status VX_API_CALL vxSetKernelAttribute(vx_kernel kernel, vx_enum attribute, const void *ptr, vx_size size)
{
    CHECK_OBJECT(kernel, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) kernel;
    CHECK_DISPATCH(platform, vxSetKernelAttribute, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxSetKernelAttribute(kernel, attribute, ptr, size);
}

vx_parameter VX_API_CALL vxGetKernelParameterByIndex(vx_kernel kernel, vx_uint32 index)
{
    CHECK_OBJECT(kernel, NULL);
    vx_platform platform = *(vx_platform *) kernel;
    CHECK_DISPATCH(platform, vxGetKernelParameterByIndex, NULL);
    return platform->vxGetKernelParameterByIndex(kernel, index);
}

vx_graph VX_API_CALL vxCreateGraph(vx_context context)
{
    CHECK_OBJECT(context, NULL);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxCreateGraph, NULL);
    return platform->vxCreateGraph(context);
}

vx_status VX_API_CALL vxReleaseGraph(vx_graph *graph)
{
    CHECK_OBJECT_PTR(graph, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) *graph;
    CHECK_DISPATCH(platform, vxReleaseGraph, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxReleaseGraph(graph);
}

vx_status VX_API_CALL vxVerifyGraph(vx_graph graph)
{
    CHECK_OBJECT(graph, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxVerifyGraph, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxVerifyGraph(graph);
}

vx_status VX_API_CALL vxProcessGraph(vx_graph graph)
{
    CHECK_OBJECT(graph, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxProcessGraph, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxProcessGraph(graph);
}

vx_status VX_API_CALL vxScheduleGraph(vx_graph graph)
{
    CHECK_OBJECT(graph, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxScheduleGraph, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxScheduleGraph(graph);
}

vx_status VX_API_CALL vxWaitGraph(vx_graph graph)
{
    CHECK_OBJECT(graph, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxWaitGraph, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxWaitGraph(graph);
}

vx_status VX_API_CALL vxQueryGraph(vx_graph graph, vx_enum attribute, void *ptr, vx_size size)
{
    CHECK_OBJECT(graph, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxQueryGraph, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxQueryGraph(graph, attribute, ptr, size);
}

vx_status VX_API_CALL vxSetGraphAttribute(vx_graph graph, vx_enum attribute, const void *ptr, vx_size size)
{
    CHECK_OBJECT(graph, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxSetGraphAttribute, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxSetGraphAttribute(graph, attribute, ptr, size);
}

vx_status VX_API_CALL vxAddParameterToGraph(vx_graph graph, vx_parameter parameter)
{
    CHECK_OBJECT(graph, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxAddParameterToGraph, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxAddParameterToGraph(graph, parameter);
}

vx_status VX_API_CALL vxSetGraphParameterByIndex(vx_graph graph, vx_uint32 index, vx_reference value)
{
    CHECK_OBJECT(graph, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxSetGraphParameterByIndex, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxSetGraphParameterByIndex(graph, index, value);
}

vx_parameter VX_API_CALL vxGetGraphParameterByIndex(vx_graph graph, vx_uint32 index)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxGetGraphParameterByIndex, NULL);
    return platform->vxGetGraphParameterByIndex(graph, index);
}

vx_bool VX_API_CALL vxIsGraphVerified(vx_graph graph)
{
    CHECK_OBJECT(graph, vx_false_e);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxIsGraphVerified, vx_false_e);
    return platform->vxIsGraphVerified(graph);
}

vx_node VX_API_CALL vxCreateGenericNode(vx_graph graph, vx_kernel kernel)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxCreateGenericNode, NULL);
    return platform->vxCreateGenericNode(graph, kernel);
}

vx_status VX_API_CALL vxQueryNode(vx_node node, vx_enum attribute, void *ptr, vx_size size)
{
    CHECK_OBJECT(node, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) node;
    CHECK_DISPATCH(platform, vxQueryNode, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxQueryNode(node, attribute, ptr, size);
}

vx_status VX_API_CALL vxSetNodeAttribute(vx_node node, vx_enum attribute, const void *ptr, vx_size size)
{
    CHECK_OBJECT(node, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) node;
    CHECK_DISPATCH(platform, vxSetNodeAttribute, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxSetNodeAttribute(node, attribute, ptr, size);
}

vx_status VX_API_CALL vxReleaseNode(vx_node *node)
{
    CHECK_OBJECT_PTR(node, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) *node;
    CHECK_DISPATCH(platform, vxReleaseNode, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxReleaseNode(node);
}

vx_status VX_API_CALL vxRemoveNode(vx_node *node)
{
    CHECK_OBJECT_PTR(node, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) *node;
    CHECK_DISPATCH(platform, vxRemoveNode, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxRemoveNode(node);
}

vx_status VX_API_CALL vxAssignNodeCallback(vx_node node, vx_nodecomplete_f callback)
{
    CHECK_OBJECT(node, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) node;
    CHECK_DISPATCH(platform, vxAssignNodeCallback, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxAssignNodeCallback(node, callback);
}

vx_nodecomplete_f VX_API_CALL vxRetrieveNodeCallback(vx_node node)
{
    CHECK_OBJECT(node, NULL);
    vx_platform platform = *(vx_platform *) node;
    CHECK_DISPATCH(platform, vxRetrieveNodeCallback, NULL);
    return platform->vxRetrieveNodeCallback(node);
}

vx_status VX_API_CALL vxSetNodeTarget(vx_node node, vx_enum target_enum, const char* target_string)
{
    CHECK_OBJECT(node, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) node;
    CHECK_DISPATCH(platform, vxSetNodeTarget, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxSetNodeTarget(node, target_enum, target_string);
}

vx_status VX_API_CALL vxReplicateNode(vx_graph graph, vx_node first_node, vx_bool replicate[], vx_uint32 number_of_parameters)
{
    CHECK_OBJECT(graph, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxReplicateNode, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxReplicateNode(graph, first_node, replicate, number_of_parameters);
}

vx_parameter VX_API_CALL vxGetParameterByIndex(vx_node node, vx_uint32 index)
{
    CHECK_OBJECT(node, NULL);
    vx_platform platform = *(vx_platform *) node;
    CHECK_DISPATCH(platform, vxGetParameterByIndex, NULL);
    return platform->vxGetParameterByIndex(node, index);
}

vx_status VX_API_CALL vxReleaseParameter(vx_parameter *param)
{
    CHECK_OBJECT_PTR(param, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) *param;
    CHECK_DISPATCH(platform, vxReleaseParameter, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxReleaseParameter(param);
}

vx_status VX_API_CALL vxSetParameterByIndex(vx_node node, vx_uint32 index, vx_reference value)
{
    CHECK_OBJECT(node, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) node;
    CHECK_DISPATCH(platform, vxSetParameterByIndex, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxSetParameterByIndex(node, index, value);
}

vx_status VX_API_CALL vxSetParameterByReference(vx_parameter parameter, vx_reference value)
{
    CHECK_OBJECT(parameter, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) parameter;
    CHECK_DISPATCH(platform, vxSetParameterByReference, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxSetParameterByReference(parameter, value);
}

vx_status VX_API_CALL vxQueryParameter(vx_parameter param, vx_enum attribute, void *ptr, vx_size size)
{
    CHECK_OBJECT(param, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) param;
    CHECK_DISPATCH(platform, vxQueryParameter, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxQueryParameter(param, attribute, ptr, size);
}

vx_scalar VX_API_CALL vxCreateScalar(vx_context context, vx_enum data_type, const void *ptr)
{
    CHECK_OBJECT(context, NULL);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxCreateScalar, NULL);
    return platform->vxCreateScalar(context, data_type, ptr);
}

vx_status VX_API_CALL vxReleaseScalar(vx_scalar *scalar)
{
    CHECK_OBJECT_PTR(scalar, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) *scalar;
    CHECK_DISPATCH(platform, vxReleaseScalar, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxReleaseScalar(scalar);
}

vx_status VX_API_CALL vxQueryScalar(vx_scalar scalar, vx_enum attribute, void *ptr, vx_size size)
{
    CHECK_OBJECT(scalar, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) scalar;
    CHECK_DISPATCH(platform, vxQueryScalar, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxQueryScalar(scalar, attribute, ptr, size);
}

vx_status VX_API_CALL vxReadScalarValue(vx_scalar ref, void *ptr)
{
    CHECK_OBJECT(ref, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) ref;
    CHECK_DISPATCH(platform, vxReadScalarValue, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxReadScalarValue(ref, ptr);
}

vx_status VX_API_CALL vxWriteScalarValue(vx_scalar ref, const void *ptr)
{
    CHECK_OBJECT(ref, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) ref;
    CHECK_DISPATCH(platform, vxWriteScalarValue, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxWriteScalarValue(ref, ptr);
}

vx_status VX_API_CALL vxCopyScalar(vx_scalar scalar, void *user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    CHECK_OBJECT(scalar, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) scalar;
    CHECK_DISPATCH(platform, vxCopyScalar, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxCopyScalar(scalar, user_ptr, usage, user_mem_type);
}

vx_status VX_API_CALL vxQueryReference(vx_reference ref, vx_enum attribute, void *ptr, vx_size size)
{
    CHECK_OBJECT(ref, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) ref;
    CHECK_DISPATCH(platform, vxQueryReference, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxQueryReference(ref, attribute, ptr, size);
}

vx_status VX_API_CALL vxReleaseReference(vx_reference* ref_ptr)
{
    CHECK_OBJECT(ref_ptr, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) ref_ptr;
    CHECK_DISPATCH(platform, vxReleaseReference, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxReleaseReference(ref_ptr);
}

vx_status VX_API_CALL vxRetainReference(vx_reference ref)
{
    CHECK_OBJECT(ref, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) ref;
    CHECK_DISPATCH(platform, vxRetainReference, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxRetainReference(ref);
}

vx_status VX_API_CALL vxSetReferenceName(vx_reference ref, const vx_char *name)
{
    CHECK_OBJECT(ref, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) ref;
    CHECK_DISPATCH(platform, vxSetReferenceName, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxSetReferenceName(ref, name);
}

vx_status VX_API_CALL vxQueryDelay(vx_delay delay, vx_enum attribute, void *ptr, vx_size size)
{
    CHECK_OBJECT(delay, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) delay;
    CHECK_DISPATCH(platform, vxQueryDelay, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxQueryDelay(delay, attribute, ptr, size);
}

vx_status VX_API_CALL vxReleaseDelay(vx_delay *delay)
{
    CHECK_OBJECT_PTR(delay, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) *delay;
    CHECK_DISPATCH(platform, vxReleaseDelay, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxReleaseDelay(delay);
}

vx_delay VX_API_CALL vxCreateDelay(vx_context context, vx_reference exemplar, vx_size slots)
{
    CHECK_OBJECT(context, NULL);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxCreateDelay, NULL);
    return platform->vxCreateDelay(context, exemplar, slots);
}

vx_reference VX_API_CALL vxGetReferenceFromDelay(vx_delay delay, vx_int32 index)
{
    CHECK_OBJECT(delay, NULL);
    vx_platform platform = *(vx_platform *) delay;
    CHECK_DISPATCH(platform, vxGetReferenceFromDelay, NULL);
    return platform->vxGetReferenceFromDelay(delay, index);
}

vx_status VX_API_CALL vxAgeDelay(vx_delay delay)
{
    CHECK_OBJECT(delay, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) delay;
    CHECK_DISPATCH(platform, vxAgeDelay, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxAgeDelay(delay);
}

vx_status VX_API_CALL vxRegisterAutoAging(vx_graph graph, vx_delay delay)
{
    CHECK_OBJECT(graph, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxRegisterAutoAging, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxRegisterAutoAging(graph, delay);
}

void VX_API_CALL vxAddLogEntry(vx_reference ref, vx_status status, const char *message, ...)
{
    CHECK_OBJECT_VOID(ref);
    vx_platform platform = *(vx_platform *) ref;
    CHECK_DISPATCH_VOID(platform, vxAddLogEntry);
    va_list ap;
    vx_char _message[VX_MAX_LOG_MESSAGE_LEN];
    va_start(ap, message);
    vsnprintf(_message, VX_MAX_LOG_MESSAGE_LEN, message, ap);
    _message[VX_MAX_LOG_MESSAGE_LEN - 1] = 0; // for MSVC which is not C99 compliant
    va_end(ap);
    platform->vxAddLogEntry(ref, status, _message);
}

void VX_API_CALL vxRegisterLogCallback(vx_context context, vx_log_callback_f callback, vx_bool reentrant)
{
    CHECK_OBJECT_VOID(context);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH_VOID(platform, vxRegisterLogCallback);
    platform->vxRegisterLogCallback(context, callback, reentrant);
}

vx_lut VX_API_CALL vxCreateLUT(vx_context context, vx_enum data_type, vx_size count)
{
    CHECK_OBJECT(context, NULL);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxCreateLUT, NULL);
    return platform->vxCreateLUT(context, data_type, count);
}

vx_status VX_API_CALL vxReleaseLUT(vx_lut *lut)
{
    CHECK_OBJECT_PTR(lut, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) *lut;
    CHECK_DISPATCH(platform, vxReleaseLUT, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxReleaseLUT(lut);
}

vx_status VX_API_CALL vxQueryLUT(vx_lut lut, vx_enum attribute, void *ptr, vx_size size)
{
    CHECK_OBJECT(lut, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) lut;
    CHECK_DISPATCH(platform, vxQueryLUT, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxQueryLUT(lut, attribute, ptr, size);
}

vx_status VX_API_CALL vxAccessLUT(vx_lut lut, void **ptr, vx_enum usage)
{
    CHECK_OBJECT(lut, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) lut;
    CHECK_DISPATCH(platform, vxAccessLUT, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxAccessLUT(lut, ptr, usage);
}

vx_status VX_API_CALL vxCommitLUT(vx_lut lut, const void *ptr)
{
    CHECK_OBJECT(lut, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) lut;
    CHECK_DISPATCH(platform, vxCommitLUT, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxCommitLUT(lut, ptr);
}

vx_status VX_API_CALL vxCopyLUT(vx_lut lut, void *user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    CHECK_OBJECT(lut, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) lut;
    CHECK_DISPATCH(platform, vxCopyLUT, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxCopyLUT(lut, user_ptr, usage, user_mem_type);
}

vx_status VX_API_CALL vxMapLUT(vx_lut lut, vx_map_id *map_id, void **ptr, vx_enum usage, vx_enum mem_type, vx_bitfield flags)
{
    CHECK_OBJECT(lut, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) lut;
    CHECK_DISPATCH(platform, vxMapLUT, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxMapLUT(lut, map_id, ptr, usage, mem_type, flags);
}

vx_status VX_API_CALL vxUnmapLUT(vx_lut lut, vx_map_id map_id)
{
    CHECK_OBJECT(lut, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) lut;
    CHECK_DISPATCH(platform, vxUnmapLUT, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxUnmapLUT(lut, map_id);
}

vx_distribution VX_API_CALL vxCreateDistribution(vx_context context, vx_size numBins, vx_int32 offset, vx_uint32 range)
{
    CHECK_OBJECT(context, NULL);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxCreateDistribution, NULL);
    return platform->vxCreateDistribution(context, numBins, offset, range);
}

vx_status VX_API_CALL vxReleaseDistribution(vx_distribution *distribution)
{
    CHECK_OBJECT_PTR(distribution, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) *distribution;
    CHECK_DISPATCH(platform, vxReleaseDistribution, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxReleaseDistribution(distribution);
}

vx_status VX_API_CALL vxQueryDistribution(vx_distribution distribution, vx_enum attribute, void *ptr, vx_size size)
{
    CHECK_OBJECT(distribution, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) distribution;
    CHECK_DISPATCH(platform, vxQueryDistribution, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxQueryDistribution(distribution, attribute, ptr, size);
}

vx_status VX_API_CALL vxAccessDistribution(vx_distribution distribution, void **ptr, vx_enum usage)
{
    CHECK_OBJECT(distribution, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) distribution;
    CHECK_DISPATCH(platform, vxAccessDistribution, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxAccessDistribution(distribution, ptr, usage);
}

vx_status VX_API_CALL vxCommitDistribution(vx_distribution distribution, const void * ptr)
{
    CHECK_OBJECT(distribution, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) distribution;
    CHECK_DISPATCH(platform, vxCommitDistribution, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxCommitDistribution(distribution, ptr);
}

vx_status VX_API_CALL vxCopyDistribution(vx_distribution distribution, void *user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    CHECK_OBJECT(distribution, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) distribution;
    CHECK_DISPATCH(platform, vxCopyDistribution, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxCopyDistribution(distribution, user_ptr, usage, user_mem_type);
}

vx_status VX_API_CALL vxMapDistribution(vx_distribution distribution, vx_map_id *map_id, void **ptr, vx_enum usage, vx_enum mem_type, vx_bitfield flags)
{
    CHECK_OBJECT(distribution, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) distribution;
    CHECK_DISPATCH(platform, vxMapDistribution, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxMapDistribution(distribution, map_id, ptr, usage, mem_type, flags);
}

vx_status VX_API_CALL vxUnmapDistribution(vx_distribution distribution, vx_map_id map_id)
{
    CHECK_OBJECT(distribution, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) distribution;
    CHECK_DISPATCH(platform, vxUnmapDistribution, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxUnmapDistribution(distribution, map_id);
}

vx_threshold VX_API_CALL vxCreateThreshold(vx_context c, vx_enum thresh_type, vx_enum data_type)
{
    CHECK_OBJECT(c, NULL);
    vx_platform platform = *(vx_platform *) c;
    CHECK_DISPATCH(platform, vxCreateThreshold, NULL);
    return platform->vxCreateThreshold(c, thresh_type, data_type);
}

vx_status VX_API_CALL vxReleaseThreshold(vx_threshold *thresh)
{
    CHECK_OBJECT_PTR(thresh, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) *thresh;
    CHECK_DISPATCH(platform, vxReleaseThreshold, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxReleaseThreshold(thresh);
}

vx_status VX_API_CALL vxSetThresholdAttribute(vx_threshold thresh, vx_enum attribute, const void *ptr, vx_size size)
{
    CHECK_OBJECT(thresh, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) thresh;
    CHECK_DISPATCH(platform, vxSetThresholdAttribute, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxSetThresholdAttribute(thresh, attribute, ptr, size);
}

vx_status VX_API_CALL vxQueryThreshold(vx_threshold thresh, vx_enum attribute, void *ptr, vx_size size)
{
    CHECK_OBJECT(thresh, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) thresh;
    CHECK_DISPATCH(platform, vxQueryThreshold, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxQueryThreshold(thresh, attribute, ptr, size);
}

vx_matrix VX_API_CALL vxCreateMatrix(vx_context c, vx_enum data_type, vx_size columns, vx_size rows)
{
    CHECK_OBJECT(c, NULL);
    vx_platform platform = *(vx_platform *) c;
    CHECK_DISPATCH(platform, vxCreateMatrix, NULL);
    return platform->vxCreateMatrix(c, data_type, columns, rows);
}

vx_status VX_API_CALL vxReleaseMatrix(vx_matrix *mat)
{
    CHECK_OBJECT_PTR(mat, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) *mat;
    CHECK_DISPATCH(platform, vxReleaseMatrix, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxReleaseMatrix(mat);
}

vx_status VX_API_CALL vxQueryMatrix(vx_matrix mat, vx_enum attribute, void *ptr, vx_size size)
{
    CHECK_OBJECT(mat, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) mat;
    CHECK_DISPATCH(platform, vxQueryMatrix, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxQueryMatrix(mat, attribute, ptr, size);
}

vx_status VX_API_CALL vxReadMatrix(vx_matrix mat, void *array)
{
    CHECK_OBJECT(mat, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) mat;
    CHECK_DISPATCH(platform, vxReadMatrix, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxReadMatrix(mat, array);
}

vx_status VX_API_CALL vxWriteMatrix(vx_matrix mat, const void *array)
{
    CHECK_OBJECT(mat, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) mat;
    CHECK_DISPATCH(platform, vxWriteMatrix, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxWriteMatrix(mat, array);
}

vx_status VX_API_CALL vxCopyMatrix(vx_matrix matrix, void *user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    CHECK_OBJECT(matrix, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) matrix;
    CHECK_DISPATCH(platform, vxCopyMatrix, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxCopyMatrix(matrix, user_ptr, usage, user_mem_type);
}

vx_matrix VX_API_CALL vxCreateMatrixFromPattern(vx_context context, vx_enum pattern, vx_size columns, vx_size rows)
{
    CHECK_OBJECT(context, NULL);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxCreateMatrixFromPattern, NULL);
    return platform->vxCreateMatrixFromPattern(context, pattern, columns, rows);
}

vx_convolution VX_API_CALL vxCreateConvolution(vx_context context, vx_size columns, vx_size rows)
{
    CHECK_OBJECT(context, NULL);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxCreateConvolution, NULL);
    return platform->vxCreateConvolution(context, columns, rows);
}

vx_status VX_API_CALL vxReleaseConvolution(vx_convolution *conv)
{
    CHECK_OBJECT_PTR(conv, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) *conv;
    CHECK_DISPATCH(platform, vxReleaseConvolution, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxReleaseConvolution(conv);
}

vx_status VX_API_CALL vxQueryConvolution(vx_convolution conv, vx_enum attribute, void *ptr, vx_size size)
{
    CHECK_OBJECT(conv, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) conv;
    CHECK_DISPATCH(platform, vxQueryConvolution, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxQueryConvolution(conv, attribute, ptr, size);
}

vx_status VX_API_CALL vxSetConvolutionAttribute(vx_convolution conv, vx_enum attribute, const void *ptr, vx_size size)
{
    CHECK_OBJECT(conv, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) conv;
    CHECK_DISPATCH(platform, vxSetConvolutionAttribute, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxSetConvolutionAttribute(conv, attribute, ptr, size);
}

vx_status VX_API_CALL vxReadConvolutionCoefficients(vx_convolution conv, vx_int16 *array)
{
    CHECK_OBJECT(conv, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) conv;
    CHECK_DISPATCH(platform, vxReadConvolutionCoefficients, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxReadConvolutionCoefficients(conv, array);
}

vx_status VX_API_CALL vxWriteConvolutionCoefficients(vx_convolution conv, const vx_int16 *array)
{
    CHECK_OBJECT(conv, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) conv;
    CHECK_DISPATCH(platform, vxWriteConvolutionCoefficients, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxWriteConvolutionCoefficients(conv, array);
}

vx_status VX_API_CALL vxCopyConvolutionCoefficients(vx_convolution conv, void *user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    CHECK_OBJECT(conv, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) conv;
    CHECK_DISPATCH(platform, vxCopyConvolutionCoefficients, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxCopyConvolutionCoefficients(conv, user_ptr, usage, user_mem_type);
}

vx_pyramid VX_API_CALL vxCreatePyramid(vx_context context, vx_size levels, vx_float32 scale, vx_uint32 width, vx_uint32 height, vx_df_image format)
{
    CHECK_OBJECT(context, NULL);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxCreatePyramid, NULL);
    return platform->vxCreatePyramid(context, levels, scale, width, height, format);
}

vx_pyramid VX_API_CALL vxCreateVirtualPyramid(vx_graph graph, vx_size levels, vx_float32 scale, vx_uint32 width, vx_uint32 height, vx_df_image format)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxCreateVirtualPyramid, NULL);
    return platform->vxCreateVirtualPyramid(graph, levels, scale, width, height, format);
}

vx_status VX_API_CALL vxReleasePyramid(vx_pyramid *pyr)
{
    CHECK_OBJECT_PTR(pyr, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) *pyr;
    CHECK_DISPATCH(platform, vxReleasePyramid, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxReleasePyramid(pyr);
}

vx_status VX_API_CALL vxQueryPyramid(vx_pyramid pyr, vx_enum attribute, void *ptr, vx_size size)
{
    CHECK_OBJECT(pyr, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) pyr;
    CHECK_DISPATCH(platform, vxQueryPyramid, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxQueryPyramid(pyr, attribute, ptr, size);
}

vx_image VX_API_CALL vxGetPyramidLevel(vx_pyramid pyr, vx_uint32 index)
{
    CHECK_OBJECT(pyr, NULL);
    vx_platform platform = *(vx_platform *) pyr;
    CHECK_DISPATCH(platform, vxGetPyramidLevel, NULL);
    return platform->vxGetPyramidLevel(pyr, index);
}

vx_remap VX_API_CALL vxCreateRemap(vx_context context, vx_uint32 src_width, vx_uint32 src_height, vx_uint32 dst_width, vx_uint32 dst_height)
{
    CHECK_OBJECT(context, NULL);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxCreateRemap, NULL);
    return platform->vxCreateRemap(context, src_width, src_height, dst_width, dst_height);
}

vx_status VX_API_CALL vxReleaseRemap(vx_remap *table)
{
    CHECK_OBJECT_PTR(table, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) *table;
    CHECK_DISPATCH(platform, vxReleaseRemap, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxReleaseRemap(table);
}

vx_status VX_API_CALL vxSetRemapPoint(vx_remap table, vx_uint32 dst_x, vx_uint32 dst_y, vx_float32 src_x, vx_float32 src_y)
{
    CHECK_OBJECT(table, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) table;
    CHECK_DISPATCH(platform, vxSetRemapPoint, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxSetRemapPoint(table, dst_x, dst_y, src_x, src_y);
}

vx_status VX_API_CALL vxGetRemapPoint(vx_remap table, vx_uint32 dst_x, vx_uint32 dst_y, vx_float32 *src_x, vx_float32 *src_y)
{
    CHECK_OBJECT(table, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) table;
    CHECK_DISPATCH(platform, vxGetRemapPoint, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxGetRemapPoint(table, dst_x, dst_y, src_x, src_y);
}

vx_status VX_API_CALL vxQueryRemap(vx_remap r, vx_enum attribute, void *ptr, vx_size size)
{
    CHECK_OBJECT(r, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) r;
    CHECK_DISPATCH(platform, vxQueryRemap, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxQueryRemap(r, attribute, ptr, size);
}

vx_array VX_API_CALL vxCreateArray(vx_context context, vx_enum item_type, vx_size capacity)
{
    CHECK_OBJECT(context, NULL);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxCreateArray, NULL);
    return platform->vxCreateArray(context, item_type, capacity);
}

vx_array VX_API_CALL vxCreateVirtualArray(vx_graph graph, vx_enum item_type, vx_size capacity)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxCreateVirtualArray, NULL);
    return platform->vxCreateVirtualArray(graph, item_type, capacity);
}

vx_status VX_API_CALL vxReleaseArray(vx_array *arr)
{
    CHECK_OBJECT_PTR(arr, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) *arr;
    CHECK_DISPATCH(platform, vxReleaseArray, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxReleaseArray(arr);
}

vx_status VX_API_CALL vxQueryArray(vx_array arr, vx_enum attribute, void *ptr, vx_size size)
{
    CHECK_OBJECT(arr, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) arr;
    CHECK_DISPATCH(platform, vxQueryArray, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxQueryArray(arr, attribute, ptr, size);
}

vx_status VX_API_CALL vxAddArrayItems(vx_array arr, vx_size count, const void *ptr, vx_size stride)
{
    CHECK_OBJECT(arr, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) arr;
    CHECK_DISPATCH(platform, vxAddArrayItems, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxAddArrayItems(arr, count, ptr, stride);
}

vx_status VX_API_CALL vxTruncateArray(vx_array arr, vx_size new_num_items)
{
    CHECK_OBJECT(arr, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) arr;
    CHECK_DISPATCH(platform, vxTruncateArray, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxTruncateArray(arr, new_num_items);
}

vx_status VX_API_CALL vxAccessArrayRange(vx_array arr, vx_size start, vx_size end, vx_size *stride, void **ptr, vx_enum usage)
{
    CHECK_OBJECT(arr, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) arr;
    CHECK_DISPATCH(platform, vxAccessArrayRange, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxAccessArrayRange(arr, start, end, stride, ptr, usage);
}

vx_status VX_API_CALL vxCommitArrayRange(vx_array arr, vx_size start, vx_size end, const void *ptr)
{
    CHECK_OBJECT(arr, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) arr;
    CHECK_DISPATCH(platform, vxCommitArrayRange, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxCommitArrayRange(arr, start, end, ptr);
}

vx_status VX_API_CALL vxCopyArrayRange(vx_array array, vx_size range_start, vx_size range_end, vx_size user_stride, void *user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    CHECK_OBJECT(array, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) array;
    CHECK_DISPATCH(platform, vxCopyArrayRange, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxCopyArrayRange(array, range_start, range_end, user_stride, user_ptr, usage, user_mem_type);
}

vx_status VX_API_CALL vxMapArrayRange(vx_array array, vx_size range_start, vx_size range_end, vx_map_id *map_id, vx_size *stride, void **ptr, vx_enum usage, vx_enum mem_type, vx_uint32 flags)
{
    CHECK_OBJECT(array, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) array;
    CHECK_DISPATCH(platform, vxMapArrayRange, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxMapArrayRange(array, range_start, range_end, map_id, stride, ptr, usage, mem_type, flags);
}

vx_status VX_API_CALL vxUnmapArrayRange(vx_array array, vx_map_id map_id)
{
    CHECK_OBJECT(array, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) array;
    CHECK_DISPATCH(platform, vxUnmapArrayRange, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxUnmapArrayRange(array, map_id);
}

vx_array VX_API_CALL vxCreateObjectArray(vx_context context, vx_reference exemplar, vx_size count)
{
    CHECK_OBJECT(context, NULL);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxCreateObjectArray, NULL);
    return platform->vxCreateObjectArray(context, exemplar, count);
}

vx_array VX_API_CALL vxCreateVirtualObjectArray(vx_graph graph, vx_reference exemplar, vx_size count)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxCreateVirtualObjectArray, NULL);
    return platform->vxCreateVirtualObjectArray(graph, exemplar, count);
}

vx_reference VX_API_CALL vxGetObjectArrayItem(vx_object_array arr, vx_uint32 index)
{
    CHECK_OBJECT(arr, NULL);
    vx_platform platform = *(vx_platform *) arr;
    CHECK_DISPATCH(platform, vxGetObjectArrayItem, NULL);
    return platform->vxGetObjectArrayItem(arr, index);
}

vx_status VX_API_CALL vxReleaseObjectArray(vx_object_array *arr)
{
    CHECK_OBJECT_PTR(arr, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) *arr;
    CHECK_DISPATCH(platform, vxReleaseObjectArray, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxReleaseObjectArray(arr);
}

vx_status VX_API_CALL vxQueryObjectArray(vx_object_array arr, vx_enum attribute, void *ptr, vx_size size)
{
    CHECK_OBJECT(arr, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) arr;
    CHECK_DISPATCH(platform, vxQueryObjectArray, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxQueryObjectArray(arr, attribute, ptr, size);
}

vx_status VX_API_CALL vxSetMetaFormatAttribute(vx_meta_format meta, vx_enum attribute, const void *ptr, vx_size size)
{
    CHECK_OBJECT(meta, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) meta;
    CHECK_DISPATCH(platform, vxSetMetaFormatAttribute, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxSetMetaFormatAttribute(meta, attribute, ptr, size);
}

vx_status VX_API_CALL vxSetMetaFormatFromReference(vx_meta_format meta, vx_reference exemplar)
{
    CHECK_OBJECT(meta, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) meta;
    CHECK_DISPATCH(platform, vxSetMetaFormatFromReference, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxSetMetaFormatFromReference(meta, exemplar);
}

vx_node VX_API_CALL vxColorConvertNode(vx_graph graph, vx_image input, vx_image output)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxColorConvertNode, NULL);
    return platform->vxColorConvertNode(graph, input, output);
}

vx_node VX_API_CALL vxChannelExtractNode(vx_graph graph, vx_image input, vx_enum channel, vx_image output)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxChannelExtractNode, NULL);
    return platform->vxChannelExtractNode(graph, input, channel, output);
}

vx_node VX_API_CALL vxChannelCombineNode(vx_graph graph, vx_image plane0, vx_image plane1, vx_image plane2, vx_image plane3, vx_image output)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxChannelCombineNode, NULL);
    return platform->vxChannelCombineNode(graph, plane0, plane1, plane2, plane3, output);
}

vx_node VX_API_CALL vxPhaseNode(vx_graph graph, vx_image grad_x, vx_image grad_y, vx_image orientation)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxPhaseNode, NULL);
    return platform->vxPhaseNode(graph, grad_x, grad_y, orientation);
}

vx_node VX_API_CALL vxSobel3x3Node(vx_graph graph, vx_image input, vx_image output_x, vx_image output_y)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxSobel3x3Node, NULL);
    return platform->vxSobel3x3Node(graph, input, output_x, output_y);
}

vx_node VX_API_CALL vxMagnitudeNode(vx_graph graph, vx_image grad_x, vx_image grad_y, vx_image mag)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxMagnitudeNode, NULL);
    return platform->vxMagnitudeNode(graph, grad_x, grad_y, mag);
}

vx_node VX_API_CALL vxScaleImageNode(vx_graph graph, vx_image src, vx_image dst, vx_enum type)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxScaleImageNode, NULL);
    return platform->vxScaleImageNode(graph, src, dst, type);
}

vx_node VX_API_CALL vxTableLookupNode(vx_graph graph, vx_image input, vx_lut lut, vx_image output)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxTableLookupNode, NULL);
    return platform->vxTableLookupNode(graph, input, lut, output);
}

vx_node VX_API_CALL vxHistogramNode(vx_graph graph, vx_image input, vx_distribution distribution)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxHistogramNode, NULL);
    return platform->vxHistogramNode(graph, input, distribution);
}

vx_node VX_API_CALL vxEqualizeHistNode(vx_graph graph, vx_image input, vx_image output)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxEqualizeHistNode, NULL);
    return platform->vxEqualizeHistNode(graph, input, output);
}

vx_node VX_API_CALL vxAbsDiffNode(vx_graph graph, vx_image in1, vx_image in2, vx_image out)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxAbsDiffNode, NULL);
    return platform->vxAbsDiffNode(graph, in1, in2, out);
}

vx_node VX_API_CALL vxMeanStdDevNode(vx_graph graph, vx_image input, vx_scalar mean, vx_scalar stddev)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxMeanStdDevNode, NULL);
    return platform->vxMeanStdDevNode(graph, input, mean, stddev);
}

vx_node VX_API_CALL vxThresholdNode(vx_graph graph, vx_image input, vx_threshold thresh, vx_image output)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxThresholdNode, NULL);
    return platform->vxThresholdNode(graph, input, thresh, output);
}

vx_node VX_API_CALL vxIntegralImageNode(vx_graph graph, vx_image input, vx_image output)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxIntegralImageNode, NULL);
    return platform->vxIntegralImageNode(graph, input, output);
}

vx_node VX_API_CALL vxErode3x3Node(vx_graph graph, vx_image input, vx_image output)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxErode3x3Node, NULL);
    return platform->vxErode3x3Node(graph, input, output);
}

vx_node VX_API_CALL vxDilate3x3Node(vx_graph graph, vx_image input, vx_image output)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxDilate3x3Node, NULL);
    return platform->vxDilate3x3Node(graph, input, output);
}

vx_node VX_API_CALL vxMedian3x3Node(vx_graph graph, vx_image input, vx_image output)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxMedian3x3Node, NULL);
    return platform->vxMedian3x3Node(graph, input, output);
}

vx_node VX_API_CALL vxBox3x3Node(vx_graph graph, vx_image input, vx_image output)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxBox3x3Node, NULL);
    return platform->vxBox3x3Node(graph, input, output);
}

vx_node VX_API_CALL vxGaussian3x3Node(vx_graph graph, vx_image input, vx_image output)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxGaussian3x3Node, NULL);
    return platform->vxGaussian3x3Node(graph, input, output);
}

vx_node VX_API_CALL vxNonLinearFilterNode(vx_graph graph, vx_enum function, vx_image input, vx_matrix mask, vx_image output)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxNonLinearFilterNode, NULL);
    return platform->vxNonLinearFilterNode(graph, function, input, mask, output);
}

vx_node VX_API_CALL vxConvolveNode(vx_graph graph, vx_image input, vx_convolution conv, vx_image output)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxConvolveNode, NULL);
    return platform->vxConvolveNode(graph, input, conv, output);
}

vx_node VX_API_CALL vxGaussianPyramidNode(vx_graph graph, vx_image input, vx_pyramid gaussian)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxGaussianPyramidNode, NULL);
    return platform->vxGaussianPyramidNode(graph, input, gaussian);
}

vx_node VX_API_CALL vxLaplacianPyramidNode(vx_graph graph, vx_image input, vx_pyramid laplacian, vx_image output)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxLaplacianPyramidNode, NULL);
    return platform->vxLaplacianPyramidNode(graph, input, laplacian, output);
}

vx_node VX_API_CALL vxLaplacianReconstructNode(vx_graph graph, vx_pyramid laplacian, vx_image input, vx_image output)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxLaplacianReconstructNode, NULL);
    return platform->vxLaplacianReconstructNode(graph, laplacian, input, output);
}

vx_node VX_API_CALL vxAccumulateImageNode(vx_graph graph, vx_image input, vx_image accum)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxAccumulateImageNode, NULL);
    return platform->vxAccumulateImageNode(graph, input, accum);
}

vx_node VX_API_CALL vxAccumulateWeightedImageNode(vx_graph graph, vx_image input, vx_scalar alpha, vx_image accum)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxAccumulateWeightedImageNode, NULL);
    return platform->vxAccumulateWeightedImageNode(graph, input, alpha, accum);
}

vx_node VX_API_CALL vxAccumulateSquareImageNode(vx_graph graph, vx_image input, vx_scalar shift, vx_image accum)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxAccumulateSquareImageNode, NULL);
    return platform->vxAccumulateSquareImageNode(graph, input, shift, accum);
}

vx_node VX_API_CALL vxMinMaxLocNode(vx_graph graph, vx_image input, vx_scalar minVal, vx_scalar maxVal, vx_array minLoc, vx_array maxLoc, vx_scalar minCount, vx_scalar maxCount)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxMinMaxLocNode, NULL);
    return platform->vxMinMaxLocNode(graph, input, minVal, maxVal, minLoc, maxLoc, minCount, maxCount);
}

vx_node VX_API_CALL vxAndNode(vx_graph graph, vx_image in1, vx_image in2, vx_image out)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxAndNode, NULL);
    return platform->vxAndNode(graph, in1, in2, out);
}

vx_node VX_API_CALL vxOrNode(vx_graph graph, vx_image in1, vx_image in2, vx_image out)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxOrNode, NULL);
    return platform->vxOrNode(graph, in1, in2, out);
}

vx_node VX_API_CALL vxXorNode(vx_graph graph, vx_image in1, vx_image in2, vx_image out)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxXorNode, NULL);
    return platform->vxXorNode(graph, in1, in2, out);
}

vx_node VX_API_CALL vxNotNode(vx_graph graph, vx_image input, vx_image output)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxNotNode, NULL);
    return platform->vxNotNode(graph, input, output);
}

vx_node VX_API_CALL vxMultiplyNode(vx_graph graph, vx_image in1, vx_image in2, vx_scalar scale, vx_enum overflow_policy, vx_enum rounding_policy, vx_image out)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxMultiplyNode, NULL);
    return platform->vxMultiplyNode(graph, in1, in2, scale, overflow_policy, rounding_policy, out);
}

vx_node VX_API_CALL vxAddNode(vx_graph graph, vx_image in1, vx_image in2, vx_enum policy, vx_image out)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxAddNode, NULL);
    return platform->vxAddNode(graph, in1, in2, policy, out);
}

vx_node VX_API_CALL vxSubtractNode(vx_graph graph, vx_image in1, vx_image in2, vx_enum policy, vx_image out)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxSubtractNode, NULL);
    return platform->vxSubtractNode(graph, in1, in2, policy, out);
}

vx_node VX_API_CALL vxConvertDepthNode(vx_graph graph, vx_image input, vx_image output, vx_enum policy, vx_scalar shift)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxConvertDepthNode, NULL);
    return platform->vxConvertDepthNode(graph, input, output, policy, shift);
}

vx_node VX_API_CALL vxCannyEdgeDetectorNode(vx_graph graph, vx_image input, vx_threshold hyst, vx_int32 gradient_size, vx_enum norm_type, vx_image output)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxCannyEdgeDetectorNode, NULL);
    return platform->vxCannyEdgeDetectorNode(graph, input, hyst, gradient_size, norm_type, output);
}

vx_node VX_API_CALL vxWarpAffineNode(vx_graph graph, vx_image input, vx_matrix matrix, vx_enum type, vx_image output)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxWarpAffineNode, NULL);
    return platform->vxWarpAffineNode(graph, input, matrix, type, output);
}

vx_node VX_API_CALL vxWarpPerspectiveNode(vx_graph graph, vx_image input, vx_matrix matrix, vx_enum type, vx_image output)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxWarpPerspectiveNode, NULL);
    return platform->vxWarpPerspectiveNode(graph, input, matrix, type, output);
}

vx_node VX_API_CALL vxHarrisCornersNode(vx_graph graph, vx_image input, vx_scalar strength_thresh, vx_scalar min_distance, vx_scalar sensitivity, vx_int32 gradient_size, vx_int32 block_size, vx_array corners, vx_scalar num_corners)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxHarrisCornersNode, NULL);
    return platform->vxHarrisCornersNode(graph, input, strength_thresh, min_distance, sensitivity, gradient_size, block_size, corners, num_corners);
}

vx_node VX_API_CALL vxFastCornersNode(vx_graph graph, vx_image input, vx_scalar strength_thresh, vx_bool nonmax_suppression, vx_array corners, vx_scalar num_corners)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxFastCornersNode, NULL);
    return platform->vxFastCornersNode(graph, input, strength_thresh, nonmax_suppression, corners, num_corners);
}

vx_node VX_API_CALL vxOpticalFlowPyrLKNode(vx_graph graph, vx_pyramid old_images, vx_pyramid new_images, vx_array old_points, vx_array new_points_estimates, vx_array new_points, vx_enum termination, vx_scalar epsilon, vx_scalar num_iterations, vx_scalar use_initial_estimate, vx_size window_dimension)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxOpticalFlowPyrLKNode, NULL);
    return platform->vxOpticalFlowPyrLKNode(graph, old_images, new_images, old_points, new_points_estimates, new_points, termination, epsilon, num_iterations, use_initial_estimate, window_dimension);
}

vx_node VX_API_CALL vxRemapNode(vx_graph graph, vx_image input, vx_remap table, vx_enum policy, vx_image output)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxRemapNode, NULL);
    return platform->vxRemapNode(graph, input, table, policy, output);
}

vx_node VX_API_CALL vxHalfScaleGaussianNode(vx_graph graph, vx_image input, vx_image output, vx_int32 kernel_size)
{
    CHECK_OBJECT(graph, NULL);
    vx_platform platform = *(vx_platform *) graph;
    CHECK_DISPATCH(platform, vxHalfScaleGaussianNode, NULL);
    return platform->vxHalfScaleGaussianNode(graph, input, output, kernel_size);
}

vx_status VX_API_CALL vxuColorConvert(vx_context context, vx_image input, vx_image output)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuColorConvert, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuColorConvert(context, input, output);
}

vx_status VX_API_CALL vxuChannelExtract(vx_context context, vx_image input, vx_enum channel, vx_image output)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuChannelExtract, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuChannelExtract(context, input, channel, output);
}

vx_status VX_API_CALL vxuChannelCombine(vx_context context, vx_image plane0, vx_image plane1, vx_image plane2, vx_image plane3, vx_image output)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuChannelCombine, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuChannelCombine(context, plane0, plane1, plane2, plane3, output);
}

vx_status VX_API_CALL vxuSobel3x3(vx_context context, vx_image input, vx_image output_x, vx_image output_y)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuSobel3x3, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuSobel3x3(context, input, output_x, output_y);
}

vx_status VX_API_CALL vxuMagnitude(vx_context context, vx_image grad_x, vx_image grad_y, vx_image output)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuMagnitude, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuMagnitude(context, grad_x, grad_y, output);
}

vx_status VX_API_CALL vxuPhase(vx_context context, vx_image grad_x, vx_image grad_y, vx_image output)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuPhase, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuPhase(context, grad_x, grad_y, output);
}

vx_status VX_API_CALL vxuScaleImage(vx_context context, vx_image src, vx_image dst, vx_enum type)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuScaleImage, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuScaleImage(context, src, dst, type);
}

vx_status VX_API_CALL vxuTableLookup(vx_context context, vx_image input, vx_lut lut, vx_image output)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuTableLookup, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuTableLookup(context, input, lut, output);
}

vx_status VX_API_CALL vxuHistogram(vx_context context, vx_image input, vx_distribution distribution)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuHistogram, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuHistogram(context, input, distribution);
}

vx_status VX_API_CALL vxuEqualizeHist(vx_context context, vx_image input, vx_image output)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuEqualizeHist, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuEqualizeHist(context, input, output);
}

vx_status VX_API_CALL vxuAbsDiff(vx_context context, vx_image in1, vx_image in2, vx_image out)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuAbsDiff, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuAbsDiff(context, in1, in2, out);
}

vx_status VX_API_CALL vxuMeanStdDev(vx_context context, vx_image input, vx_float32 *mean, vx_float32 *stddev)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuMeanStdDev, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuMeanStdDev(context, input, mean, stddev);
}

vx_status VX_API_CALL vxuThreshold(vx_context context, vx_image input, vx_threshold thresh, vx_image output)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuThreshold, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuThreshold(context, input, thresh, output);
}

vx_status VX_API_CALL vxuIntegralImage(vx_context context, vx_image input, vx_image output)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuIntegralImage, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuIntegralImage(context, input, output);
}

vx_status VX_API_CALL vxuErode3x3(vx_context context, vx_image input, vx_image output)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuErode3x3, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuErode3x3(context, input, output);
}

vx_status VX_API_CALL vxuDilate3x3(vx_context context, vx_image input, vx_image output)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuDilate3x3, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuDilate3x3(context, input, output);
}

vx_status VX_API_CALL vxuMedian3x3(vx_context context, vx_image input, vx_image output)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuMedian3x3, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuMedian3x3(context, input, output);
}

vx_status VX_API_CALL vxuBox3x3(vx_context context, vx_image input, vx_image output)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuBox3x3, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuBox3x3(context, input, output);
}

vx_status VX_API_CALL vxuGaussian3x3(vx_context context, vx_image input, vx_image output)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuGaussian3x3, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuGaussian3x3(context, input, output);
}

vx_status VX_API_CALL vxuNonLinearFilter(vx_context context, vx_enum function, vx_image input, vx_matrix mask, vx_image output)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuNonLinearFilter, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuNonLinearFilter(context, function, input, mask, output);
}

vx_status VX_API_CALL vxuConvolve(vx_context context, vx_image input, vx_convolution conv, vx_image output)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuConvolve, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuConvolve(context, input, conv, output);
}

vx_status VX_API_CALL vxuGaussianPyramid(vx_context context, vx_image input, vx_pyramid gaussian)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuGaussianPyramid, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuGaussianPyramid(context, input, gaussian);
}

vx_status VX_API_CALL vxuLaplacianPyramid(vx_context context, vx_image input, vx_pyramid laplacian, vx_image output)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuLaplacianPyramid, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuLaplacianPyramid(context, input, laplacian, output);
}

vx_status VX_API_CALL vxuLaplacianReconstruct(vx_context context, vx_pyramid laplacian, vx_image input, vx_image output)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuLaplacianReconstruct, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuLaplacianReconstruct(context, laplacian, input, output);
}

vx_status VX_API_CALL vxuAccumulateImage(vx_context context, vx_image input, vx_image accum)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuAccumulateImage, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuAccumulateImage(context, input, accum);
}

vx_status VX_API_CALL vxuAccumulateWeightedImage(vx_context context, vx_image input, vx_scalar scale, vx_image accum)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuAccumulateWeightedImage, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuAccumulateWeightedImage(context, input, scale, accum);
}

vx_status VX_API_CALL vxuAccumulateSquareImage(vx_context context, vx_image input, vx_scalar shift, vx_image accum)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuAccumulateSquareImage, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuAccumulateSquareImage(context, input, shift, accum);
}

vx_status VX_API_CALL vxuMinMaxLoc(vx_context context, vx_image input, vx_scalar minVal, vx_scalar maxVal, vx_array minLoc, vx_array maxLoc, vx_scalar minCount, vx_scalar maxCount)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuMinMaxLoc, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuMinMaxLoc(context, input, minVal, maxVal, minLoc, maxLoc, minCount, maxCount);
}

vx_status VX_API_CALL vxuConvertDepth(vx_context context, vx_image input, vx_image output, vx_enum policy, vx_scalar shift)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuConvertDepth, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuConvertDepth(context, input, output, policy, shift);
}

vx_status VX_API_CALL vxuCannyEdgeDetector(vx_context context, vx_image input, vx_threshold hyst, vx_int32 gradient_size, vx_enum norm_type, vx_image output)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuCannyEdgeDetector, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuCannyEdgeDetector(context, input, hyst, gradient_size, norm_type, output);
}

vx_status VX_API_CALL vxuHalfScaleGaussian(vx_context context, vx_image input, vx_image output, vx_int32 kernel_size)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuHalfScaleGaussian, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuHalfScaleGaussian(context, input, output, kernel_size);
}

vx_status VX_API_CALL vxuAnd(vx_context context, vx_image in1, vx_image in2, vx_image out)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuAnd, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuAnd(context, in1, in2, out);
}

vx_status VX_API_CALL vxuOr(vx_context context, vx_image in1, vx_image in2, vx_image out)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuOr, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuOr(context, in1, in2, out);
}

vx_status VX_API_CALL vxuXor(vx_context context, vx_image in1, vx_image in2, vx_image out)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuXor, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuXor(context, in1, in2, out);
}

vx_status VX_API_CALL vxuNot(vx_context context, vx_image input, vx_image output)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuNot, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuNot(context, input, output);
}

vx_status VX_API_CALL vxuMultiply(vx_context context, vx_image in1, vx_image in2, vx_scalar scale, vx_enum overflow_policy, vx_enum rounding_policy, vx_image out)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuMultiply, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuMultiply(context, in1, in2, scale, overflow_policy, rounding_policy, out);
}

vx_status VX_API_CALL vxuAdd(vx_context context, vx_image in1, vx_image in2, vx_enum policy, vx_image out)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuAdd, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuAdd(context, in1, in2, policy, out);
}

vx_status VX_API_CALL vxuSubtract(vx_context context, vx_image in1, vx_image in2, vx_enum policy, vx_image out)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuSubtract, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuSubtract(context, in1, in2, policy, out);
}

vx_status VX_API_CALL vxuWarpAffine(vx_context context, vx_image input, vx_matrix matrix, vx_enum type, vx_image output)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuWarpAffine, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuWarpAffine(context, input, matrix, type, output);
}

vx_status VX_API_CALL vxuWarpPerspective(vx_context context, vx_image input, vx_matrix matrix, vx_enum type, vx_image output)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuWarpPerspective, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuWarpPerspective(context, input, matrix, type, output);
}

vx_status VX_API_CALL vxuHarrisCorners(vx_context context, vx_image input, vx_scalar strength_thresh, vx_scalar min_distance, vx_scalar sensitivity, vx_int32 gradient_size, vx_int32 block_size, vx_array corners, vx_scalar num_corners)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuHarrisCorners, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuHarrisCorners(context, input, strength_thresh, min_distance, sensitivity, gradient_size, block_size, corners, num_corners);
}

vx_status VX_API_CALL vxuFastCorners(vx_context context, vx_image input, vx_scalar strength_thresh, vx_bool nonmax_suppression, vx_array corners, vx_scalar num_corners)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuFastCorners, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuFastCorners(context, input, strength_thresh, nonmax_suppression, corners, num_corners);
}

vx_status VX_API_CALL vxuOpticalFlowPyrLK(vx_context context, vx_pyramid old_images, vx_pyramid new_images, vx_array old_points, vx_array new_points_estimates, vx_array new_points, vx_enum termination, vx_scalar epsilon, vx_scalar num_iterations, vx_scalar use_initial_estimate, vx_size window_dimension)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuOpticalFlowPyrLK, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuOpticalFlowPyrLK(context, old_images, new_images, old_points, new_points_estimates, new_points, termination, epsilon, num_iterations, use_initial_estimate, window_dimension);
}

vx_status VX_API_CALL vxuRemap(vx_context context, vx_image input, vx_remap table, vx_enum policy, vx_image output)
{
    CHECK_OBJECT(context, VX_ERROR_INVALID_REFERENCE);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxuRemap, VX_ERROR_NOT_IMPLEMENTED);
    return platform->vxuRemap(context, input, table, policy, output);
}

vx_kernel VX_API_CALL vxAddKernel(vx_context context, const vx_char name[VX_MAX_KERNEL_NAME], vx_enum enumeration, vx_kernel_f func_ptr, vx_uint32 numParams, vx_kernel_input_validate_f input, vx_kernel_output_validate_f output, vx_kernel_initialize_f init, vx_kernel_deinitialize_f deinit)
{
    CHECK_OBJECT(context, NULL);
    vx_platform platform = *(vx_platform *) context;
    CHECK_DISPATCH(platform, vxAddKernel, NULL);
    return platform->vxAddKernel(context, name, enumeration, func_ptr, numParams, input, output, init, deinit);
}

static vx_uint32 vxComputePatchOffset(vx_uint32 x, vx_uint32 y, const vx_imagepatch_addressing_t *addr)
{
    return ((addr->stride_y * ((addr->scale_y * y)/VX_SCALE_UNITY)) +
            (addr->stride_x * ((addr->scale_x * x)/VX_SCALE_UNITY)));
}

void * VX_API_CALL vxFormatImagePatchAddress1d(void *ptr, vx_uint32 index, const vx_imagepatch_addressing_t *addr)
{
    vx_uint8 *new_ptr = NULL;
    if (ptr && index < addr->dim_x*addr->dim_y)
    {
        vx_uint32 x = index % addr->dim_x;
        vx_uint32 y = index / addr->dim_x;
        vx_uint32 offset = vxComputePatchOffset(x, y, addr);
        new_ptr = (vx_uint8 *)ptr;
        new_ptr = &new_ptr[offset];
    }
    return new_ptr;
}

void * VX_API_CALL vxFormatImagePatchAddress2d(void *ptr, vx_uint32 x, vx_uint32 y, const vx_imagepatch_addressing_t *addr)
{
    vx_uint8 *new_ptr = NULL;
    if (ptr && x < addr->dim_x && y < addr->dim_y)
    {
        vx_uint32 offset = vxComputePatchOffset(x, y, addr);
        new_ptr = (vx_uint8 *)ptr;
        new_ptr = &new_ptr[offset];
    }
    return new_ptr;
}

static vx_status VX_API_ENTRY vxLoadDispatchTable(vx_platform platform)
{
    platform->handle_openvx = OpenSharedLibrary(platform->path_openvx);
    if(!platform->handle_openvx)
        return VX_FAILURE;
    if(platform->path_vxu && strcmp(platform->path_vxu, platform->path_openvx) != 0) {
        platform->handle_vxu = OpenSharedLibrary(platform->path_vxu);
        if(!platform->handle_vxu)
            return VX_FAILURE;
    }
    else {
        platform->handle_vxu = platform->handle_openvx;
    }
    platform->vxCreateContextFromPlatform = (TYPE_vxCreateContextFromPlatform *) GetFunctionAddress(platform->handle_openvx, "vxCreateContextFromPlatform");
    platform->vxCreateContext = (TYPE_vxCreateContext *) GetFunctionAddress(platform->handle_openvx, "vxCreateContext");
    platform->vxReleaseContext = (TYPE_vxReleaseContext *) GetFunctionAddress(platform->handle_openvx, "vxReleaseContext");
    platform->vxGetContext = (TYPE_vxGetContext *) GetFunctionAddress(platform->handle_openvx, "vxGetContext");
    platform->vxQueryContext = (TYPE_vxQueryContext *) GetFunctionAddress(platform->handle_openvx, "vxQueryContext");
    platform->vxSetContextAttribute = (TYPE_vxSetContextAttribute *) GetFunctionAddress(platform->handle_openvx, "vxSetContextAttribute");
    platform->vxHint = (TYPE_vxHint *) GetFunctionAddress(platform->handle_openvx, "vxHint");
    platform->vxDirective = (TYPE_vxDirective *) GetFunctionAddress(platform->handle_openvx, "vxDirective");
    platform->vxGetStatus = (TYPE_vxGetStatus *) GetFunctionAddress(platform->handle_openvx, "vxGetStatus");
    platform->vxRegisterUserStruct = (TYPE_vxRegisterUserStruct *) GetFunctionAddress(platform->handle_openvx, "vxRegisterUserStruct");
    platform->vxAllocateUserKernelId = (TYPE_vxAllocateUserKernelId *) GetFunctionAddress(platform->handle_openvx, "vxAllocateUserKernelId");
    platform->vxAllocateUserKernelLibraryId = (TYPE_vxAllocateUserKernelLibraryId *) GetFunctionAddress(platform->handle_openvx, "vxAllocateUserKernelLibraryId");
    platform->vxSetImmediateModeTarget = (TYPE_vxSetImmediateModeTarget *) GetFunctionAddress(platform->handle_openvx, "vxSetImmediateModeTarget");
    platform->vxCreateImage = (TYPE_vxCreateImage *) GetFunctionAddress(platform->handle_openvx, "vxCreateImage");
    platform->vxCreateImageFromROI = (TYPE_vxCreateImageFromROI *) GetFunctionAddress(platform->handle_openvx, "vxCreateImageFromROI");
    platform->vxCreateUniformImage = (TYPE_vxCreateUniformImage *) GetFunctionAddress(platform->handle_openvx, "vxCreateUniformImage");
    platform->vxCreateVirtualImage = (TYPE_vxCreateVirtualImage *) GetFunctionAddress(platform->handle_openvx, "vxCreateVirtualImage");
    platform->vxCreateImageFromHandle = (TYPE_vxCreateImageFromHandle *) GetFunctionAddress(platform->handle_openvx, "vxCreateImageFromHandle");
    platform->vxSwapImageHandle = (TYPE_vxSwapImageHandle *) GetFunctionAddress(platform->handle_openvx, "vxSwapImageHandle");
    platform->vxQueryImage = (TYPE_vxQueryImage *) GetFunctionAddress(platform->handle_openvx, "vxQueryImage");
    platform->vxSetImageAttribute = (TYPE_vxSetImageAttribute *) GetFunctionAddress(platform->handle_openvx, "vxSetImageAttribute");
    platform->vxReleaseImage = (TYPE_vxReleaseImage *) GetFunctionAddress(platform->handle_openvx, "vxReleaseImage");
    platform->vxComputeImagePatchSize = (TYPE_vxComputeImagePatchSize *) GetFunctionAddress(platform->handle_openvx, "vxComputeImagePatchSize");
    platform->vxAccessImagePatch = (TYPE_vxAccessImagePatch *) GetFunctionAddress(platform->handle_openvx, "vxAccessImagePatch");
    platform->vxCommitImagePatch = (TYPE_vxCommitImagePatch *) GetFunctionAddress(platform->handle_openvx, "vxCommitImagePatch");
    platform->vxGetValidRegionImage = (TYPE_vxGetValidRegionImage *) GetFunctionAddress(platform->handle_openvx, "vxGetValidRegionImage");
    platform->vxCopyImagePatch = (TYPE_vxCopyImagePatch *) GetFunctionAddress(platform->handle_openvx, "vxCopyImagePatch");
    platform->vxMapImagePatch = (TYPE_vxMapImagePatch *) GetFunctionAddress(platform->handle_openvx, "vxMapImagePatch");
    platform->vxUnmapImagePatch = (TYPE_vxUnmapImagePatch *) GetFunctionAddress(platform->handle_openvx, "vxUnmapImagePatch");
    platform->vxCreateImageFromChannel = (TYPE_vxCreateImageFromChannel *) GetFunctionAddress(platform->handle_openvx, "vxCreateImageFromChannel");
    platform->vxSetImageValidRectangle = (TYPE_vxSetImageValidRectangle *) GetFunctionAddress(platform->handle_openvx, "vxSetImageValidRectangle");
    platform->vxLoadKernels = (TYPE_vxLoadKernels *) GetFunctionAddress(platform->handle_openvx, "vxLoadKernels");
    platform->vxUnloadKernels = (TYPE_vxUnloadKernels *) GetFunctionAddress(platform->handle_openvx, "vxUnloadKernels");
    platform->vxGetKernelByName = (TYPE_vxGetKernelByName *) GetFunctionAddress(platform->handle_openvx, "vxGetKernelByName");
    platform->vxGetKernelByEnum = (TYPE_vxGetKernelByEnum *) GetFunctionAddress(platform->handle_openvx, "vxGetKernelByEnum");
    platform->vxQueryKernel = (TYPE_vxQueryKernel *) GetFunctionAddress(platform->handle_openvx, "vxQueryKernel");
    platform->vxReleaseKernel = (TYPE_vxReleaseKernel *) GetFunctionAddress(platform->handle_openvx, "vxReleaseKernel");
    platform->vxAddUserKernel = (TYPE_vxAddUserKernel *) GetFunctionAddress(platform->handle_openvx, "vxAddUserKernel");
    platform->vxFinalizeKernel = (TYPE_vxFinalizeKernel *) GetFunctionAddress(platform->handle_openvx, "vxFinalizeKernel");
    platform->vxAddParameterToKernel = (TYPE_vxAddParameterToKernel *) GetFunctionAddress(platform->handle_openvx, "vxAddParameterToKernel");
    platform->vxRemoveKernel = (TYPE_vxRemoveKernel *) GetFunctionAddress(platform->handle_openvx, "vxRemoveKernel");
    platform->vxSetKernelAttribute = (TYPE_vxSetKernelAttribute *) GetFunctionAddress(platform->handle_openvx, "vxSetKernelAttribute");
    platform->vxGetKernelParameterByIndex = (TYPE_vxGetKernelParameterByIndex *) GetFunctionAddress(platform->handle_openvx, "vxGetKernelParameterByIndex");
    platform->vxCreateGraph = (TYPE_vxCreateGraph *) GetFunctionAddress(platform->handle_openvx, "vxCreateGraph");
    platform->vxReleaseGraph = (TYPE_vxReleaseGraph *) GetFunctionAddress(platform->handle_openvx, "vxReleaseGraph");
    platform->vxVerifyGraph = (TYPE_vxVerifyGraph *) GetFunctionAddress(platform->handle_openvx, "vxVerifyGraph");
    platform->vxProcessGraph = (TYPE_vxProcessGraph *) GetFunctionAddress(platform->handle_openvx, "vxProcessGraph");
    platform->vxScheduleGraph = (TYPE_vxScheduleGraph *) GetFunctionAddress(platform->handle_openvx, "vxScheduleGraph");
    platform->vxWaitGraph = (TYPE_vxWaitGraph *) GetFunctionAddress(platform->handle_openvx, "vxWaitGraph");
    platform->vxQueryGraph = (TYPE_vxQueryGraph *) GetFunctionAddress(platform->handle_openvx, "vxQueryGraph");
    platform->vxSetGraphAttribute = (TYPE_vxSetGraphAttribute *) GetFunctionAddress(platform->handle_openvx, "vxSetGraphAttribute");
    platform->vxAddParameterToGraph = (TYPE_vxAddParameterToGraph *) GetFunctionAddress(platform->handle_openvx, "vxAddParameterToGraph");
    platform->vxSetGraphParameterByIndex = (TYPE_vxSetGraphParameterByIndex *) GetFunctionAddress(platform->handle_openvx, "vxSetGraphParameterByIndex");
    platform->vxGetGraphParameterByIndex = (TYPE_vxGetGraphParameterByIndex *) GetFunctionAddress(platform->handle_openvx, "vxGetGraphParameterByIndex");
    platform->vxIsGraphVerified = (TYPE_vxIsGraphVerified *) GetFunctionAddress(platform->handle_openvx, "vxIsGraphVerified");
    platform->vxCreateGenericNode = (TYPE_vxCreateGenericNode *) GetFunctionAddress(platform->handle_openvx, "vxCreateGenericNode");
    platform->vxQueryNode = (TYPE_vxQueryNode *) GetFunctionAddress(platform->handle_openvx, "vxQueryNode");
    platform->vxSetNodeAttribute = (TYPE_vxSetNodeAttribute *) GetFunctionAddress(platform->handle_openvx, "vxSetNodeAttribute");
    platform->vxReleaseNode = (TYPE_vxReleaseNode *) GetFunctionAddress(platform->handle_openvx, "vxReleaseNode");
    platform->vxRemoveNode = (TYPE_vxRemoveNode *) GetFunctionAddress(platform->handle_openvx, "vxRemoveNode");
    platform->vxAssignNodeCallback = (TYPE_vxAssignNodeCallback *) GetFunctionAddress(platform->handle_openvx, "vxAssignNodeCallback");
    platform->vxRetrieveNodeCallback = (TYPE_vxRetrieveNodeCallback *) GetFunctionAddress(platform->handle_openvx, "vxRetrieveNodeCallback");
    platform->vxSetNodeTarget = (TYPE_vxSetNodeTarget *) GetFunctionAddress(platform->handle_openvx, "vxSetNodeTarget");
    platform->vxReplicateNode = (TYPE_vxReplicateNode *) GetFunctionAddress(platform->handle_openvx, "vxReplicateNode");
    platform->vxGetParameterByIndex = (TYPE_vxGetParameterByIndex *) GetFunctionAddress(platform->handle_openvx, "vxGetParameterByIndex");
    platform->vxReleaseParameter = (TYPE_vxReleaseParameter *) GetFunctionAddress(platform->handle_openvx, "vxReleaseParameter");
    platform->vxSetParameterByIndex = (TYPE_vxSetParameterByIndex *) GetFunctionAddress(platform->handle_openvx, "vxSetParameterByIndex");
    platform->vxSetParameterByReference = (TYPE_vxSetParameterByReference *) GetFunctionAddress(platform->handle_openvx, "vxSetParameterByReference");
    platform->vxQueryParameter = (TYPE_vxQueryParameter *) GetFunctionAddress(platform->handle_openvx, "vxQueryParameter");
    platform->vxCreateScalar = (TYPE_vxCreateScalar *) GetFunctionAddress(platform->handle_openvx, "vxCreateScalar");
    platform->vxReleaseScalar = (TYPE_vxReleaseScalar *) GetFunctionAddress(platform->handle_openvx, "vxReleaseScalar");
    platform->vxQueryScalar = (TYPE_vxQueryScalar *) GetFunctionAddress(platform->handle_openvx, "vxQueryScalar");
    platform->vxReadScalarValue = (TYPE_vxReadScalarValue *) GetFunctionAddress(platform->handle_openvx, "vxReadScalarValue");
    platform->vxWriteScalarValue = (TYPE_vxWriteScalarValue *) GetFunctionAddress(platform->handle_openvx, "vxWriteScalarValue");
    platform->vxCopyScalar = (TYPE_vxCopyScalar *) GetFunctionAddress(platform->handle_openvx, "vxCopyScalar");
    platform->vxQueryReference = (TYPE_vxQueryReference *) GetFunctionAddress(platform->handle_openvx, "vxQueryReference");
    platform->vxReleaseReference = (TYPE_vxReleaseReference *) GetFunctionAddress(platform->handle_openvx, "vxReleaseReference");
    platform->vxRetainReference = (TYPE_vxRetainReference *) GetFunctionAddress(platform->handle_openvx, "vxRetainReference");
    platform->vxSetReferenceName = (TYPE_vxSetReferenceName *) GetFunctionAddress(platform->handle_openvx, "vxSetReferenceName");
    platform->vxQueryDelay = (TYPE_vxQueryDelay *) GetFunctionAddress(platform->handle_openvx, "vxQueryDelay");
    platform->vxReleaseDelay = (TYPE_vxReleaseDelay *) GetFunctionAddress(platform->handle_openvx, "vxReleaseDelay");
    platform->vxCreateDelay = (TYPE_vxCreateDelay *) GetFunctionAddress(platform->handle_openvx, "vxCreateDelay");
    platform->vxGetReferenceFromDelay = (TYPE_vxGetReferenceFromDelay *) GetFunctionAddress(platform->handle_openvx, "vxGetReferenceFromDelay");
    platform->vxAgeDelay = (TYPE_vxAgeDelay *) GetFunctionAddress(platform->handle_openvx, "vxAgeDelay");
    platform->vxRegisterAutoAging = (TYPE_vxRegisterAutoAging *) GetFunctionAddress(platform->handle_openvx, "vxRegisterAutoAging");
    platform->vxAddLogEntry = (TYPE_vxAddLogEntry *) GetFunctionAddress(platform->handle_openvx, "vxAddLogEntry");
    platform->vxRegisterLogCallback = (TYPE_vxRegisterLogCallback *) GetFunctionAddress(platform->handle_openvx, "vxRegisterLogCallback");
    platform->vxCreateLUT = (TYPE_vxCreateLUT *) GetFunctionAddress(platform->handle_openvx, "vxCreateLUT");
    platform->vxReleaseLUT = (TYPE_vxReleaseLUT *) GetFunctionAddress(platform->handle_openvx, "vxReleaseLUT");
    platform->vxQueryLUT = (TYPE_vxQueryLUT *) GetFunctionAddress(platform->handle_openvx, "vxQueryLUT");
    platform->vxAccessLUT = (TYPE_vxAccessLUT *) GetFunctionAddress(platform->handle_openvx, "vxAccessLUT");
    platform->vxCommitLUT = (TYPE_vxCommitLUT *) GetFunctionAddress(platform->handle_openvx, "vxCommitLUT");
    platform->vxCopyLUT = (TYPE_vxCopyLUT *) GetFunctionAddress(platform->handle_openvx, "vxCopyLUT");
    platform->vxMapLUT = (TYPE_vxMapLUT *) GetFunctionAddress(platform->handle_openvx, "vxMapLUT");
    platform->vxUnmapLUT = (TYPE_vxUnmapLUT *) GetFunctionAddress(platform->handle_openvx, "vxUnmapLUT");
    platform->vxCreateDistribution = (TYPE_vxCreateDistribution *) GetFunctionAddress(platform->handle_openvx, "vxCreateDistribution");
    platform->vxReleaseDistribution = (TYPE_vxReleaseDistribution *) GetFunctionAddress(platform->handle_openvx, "vxReleaseDistribution");
    platform->vxQueryDistribution = (TYPE_vxQueryDistribution *) GetFunctionAddress(platform->handle_openvx, "vxQueryDistribution");
    platform->vxAccessDistribution = (TYPE_vxAccessDistribution *) GetFunctionAddress(platform->handle_openvx, "vxAccessDistribution");
    platform->vxCommitDistribution = (TYPE_vxCommitDistribution *) GetFunctionAddress(platform->handle_openvx, "vxCommitDistribution");
    platform->vxCopyDistribution = (TYPE_vxCopyDistribution *) GetFunctionAddress(platform->handle_openvx, "vxCopyDistribution");
    platform->vxMapDistribution = (TYPE_vxMapDistribution *) GetFunctionAddress(platform->handle_openvx, "vxMapDistribution");
    platform->vxUnmapDistribution = (TYPE_vxUnmapDistribution *) GetFunctionAddress(platform->handle_openvx, "vxUnmapDistribution");
    platform->vxCreateThreshold = (TYPE_vxCreateThreshold *) GetFunctionAddress(platform->handle_openvx, "vxCreateThreshold");
    platform->vxReleaseThreshold = (TYPE_vxReleaseThreshold *) GetFunctionAddress(platform->handle_openvx, "vxReleaseThreshold");
    platform->vxSetThresholdAttribute = (TYPE_vxSetThresholdAttribute *) GetFunctionAddress(platform->handle_openvx, "vxSetThresholdAttribute");
    platform->vxQueryThreshold = (TYPE_vxQueryThreshold *) GetFunctionAddress(platform->handle_openvx, "vxQueryThreshold");
    platform->vxCreateMatrix = (TYPE_vxCreateMatrix *) GetFunctionAddress(platform->handle_openvx, "vxCreateMatrix");
    platform->vxReleaseMatrix = (TYPE_vxReleaseMatrix *) GetFunctionAddress(platform->handle_openvx, "vxReleaseMatrix");
    platform->vxQueryMatrix = (TYPE_vxQueryMatrix *) GetFunctionAddress(platform->handle_openvx, "vxQueryMatrix");
    platform->vxReadMatrix = (TYPE_vxReadMatrix *) GetFunctionAddress(platform->handle_openvx, "vxReadMatrix");
    platform->vxWriteMatrix = (TYPE_vxWriteMatrix *) GetFunctionAddress(platform->handle_openvx, "vxWriteMatrix");
    platform->vxCopyMatrix = (TYPE_vxCopyMatrix *) GetFunctionAddress(platform->handle_openvx, "vxCopyMatrix");
    platform->vxCreateMatrixFromPattern = (TYPE_vxCreateMatrixFromPattern *) GetFunctionAddress(platform->handle_openvx, "vxCreateMatrixFromPattern");
    platform->vxCreateConvolution = (TYPE_vxCreateConvolution *) GetFunctionAddress(platform->handle_openvx, "vxCreateConvolution");
    platform->vxReleaseConvolution = (TYPE_vxReleaseConvolution *) GetFunctionAddress(platform->handle_openvx, "vxReleaseConvolution");
    platform->vxQueryConvolution = (TYPE_vxQueryConvolution *) GetFunctionAddress(platform->handle_openvx, "vxQueryConvolution");
    platform->vxSetConvolutionAttribute = (TYPE_vxSetConvolutionAttribute *) GetFunctionAddress(platform->handle_openvx, "vxSetConvolutionAttribute");
    platform->vxReadConvolutionCoefficients = (TYPE_vxReadConvolutionCoefficients *) GetFunctionAddress(platform->handle_openvx, "vxReadConvolutionCoefficients");
    platform->vxWriteConvolutionCoefficients = (TYPE_vxWriteConvolutionCoefficients *) GetFunctionAddress(platform->handle_openvx, "vxWriteConvolutionCoefficients");
    platform->vxCopyConvolutionCoefficients = (TYPE_vxCopyConvolutionCoefficients *) GetFunctionAddress(platform->handle_openvx, "vxCopyConvolutionCoefficients");
    platform->vxCreatePyramid = (TYPE_vxCreatePyramid *) GetFunctionAddress(platform->handle_openvx, "vxCreatePyramid");
    platform->vxCreateVirtualPyramid = (TYPE_vxCreateVirtualPyramid *) GetFunctionAddress(platform->handle_openvx, "vxCreateVirtualPyramid");
    platform->vxReleasePyramid = (TYPE_vxReleasePyramid *) GetFunctionAddress(platform->handle_openvx, "vxReleasePyramid");
    platform->vxQueryPyramid = (TYPE_vxQueryPyramid *) GetFunctionAddress(platform->handle_openvx, "vxQueryPyramid");
    platform->vxGetPyramidLevel = (TYPE_vxGetPyramidLevel *) GetFunctionAddress(platform->handle_openvx, "vxGetPyramidLevel");
    platform->vxCreateRemap = (TYPE_vxCreateRemap *) GetFunctionAddress(platform->handle_openvx, "vxCreateRemap");
    platform->vxReleaseRemap = (TYPE_vxReleaseRemap *) GetFunctionAddress(platform->handle_openvx, "vxReleaseRemap");
    platform->vxSetRemapPoint = (TYPE_vxSetRemapPoint *) GetFunctionAddress(platform->handle_openvx, "vxSetRemapPoint");
    platform->vxGetRemapPoint = (TYPE_vxGetRemapPoint *) GetFunctionAddress(platform->handle_openvx, "vxGetRemapPoint");
    platform->vxQueryRemap = (TYPE_vxQueryRemap *) GetFunctionAddress(platform->handle_openvx, "vxQueryRemap");
    platform->vxCreateArray = (TYPE_vxCreateArray *) GetFunctionAddress(platform->handle_openvx, "vxCreateArray");
    platform->vxCreateVirtualArray = (TYPE_vxCreateVirtualArray *) GetFunctionAddress(platform->handle_openvx, "vxCreateVirtualArray");
    platform->vxReleaseArray = (TYPE_vxReleaseArray *) GetFunctionAddress(platform->handle_openvx, "vxReleaseArray");
    platform->vxQueryArray = (TYPE_vxQueryArray *) GetFunctionAddress(platform->handle_openvx, "vxQueryArray");
    platform->vxAddArrayItems = (TYPE_vxAddArrayItems *) GetFunctionAddress(platform->handle_openvx, "vxAddArrayItems");
    platform->vxTruncateArray = (TYPE_vxTruncateArray *) GetFunctionAddress(platform->handle_openvx, "vxTruncateArray");
    platform->vxAccessArrayRange = (TYPE_vxAccessArrayRange *) GetFunctionAddress(platform->handle_openvx, "vxAccessArrayRange");
    platform->vxCommitArrayRange = (TYPE_vxCommitArrayRange *) GetFunctionAddress(platform->handle_openvx, "vxCommitArrayRange");
    platform->vxCopyArrayRange = (TYPE_vxCopyArrayRange *) GetFunctionAddress(platform->handle_openvx, "vxCopyArrayRange");
    platform->vxMapArrayRange = (TYPE_vxMapArrayRange *) GetFunctionAddress(platform->handle_openvx, "vxMapArrayRange");
    platform->vxUnmapArrayRange = (TYPE_vxUnmapArrayRange *) GetFunctionAddress(platform->handle_openvx, "vxUnmapArrayRange");
    platform->vxCreateObjectArray = (TYPE_vxCreateObjectArray *) GetFunctionAddress(platform->handle_openvx, "vxCreateObjectArray");
    platform->vxCreateVirtualObjectArray = (TYPE_vxCreateVirtualObjectArray *) GetFunctionAddress(platform->handle_openvx, "vxCreateVirtualObjectArray");
    platform->vxGetObjectArrayItem = (TYPE_vxGetObjectArrayItem *) GetFunctionAddress(platform->handle_openvx, "vxGetObjectArrayItem");
    platform->vxReleaseObjectArray = (TYPE_vxReleaseObjectArray *) GetFunctionAddress(platform->handle_openvx, "vxReleaseObjectArray");
    platform->vxQueryObjectArray = (TYPE_vxQueryObjectArray *) GetFunctionAddress(platform->handle_openvx, "vxQueryObjectArray");
    platform->vxSetMetaFormatAttribute = (TYPE_vxSetMetaFormatAttribute *) GetFunctionAddress(platform->handle_openvx, "vxSetMetaFormatAttribute");
    platform->vxSetMetaFormatFromReference = (TYPE_vxSetMetaFormatFromReference *) GetFunctionAddress(platform->handle_openvx, "vxSetMetaFormatFromReference");
    platform->vxColorConvertNode = (TYPE_vxColorConvertNode *) GetFunctionAddress(platform->handle_openvx, "vxColorConvertNode");
    platform->vxChannelExtractNode = (TYPE_vxChannelExtractNode *) GetFunctionAddress(platform->handle_openvx, "vxChannelExtractNode");
    platform->vxChannelCombineNode = (TYPE_vxChannelCombineNode *) GetFunctionAddress(platform->handle_openvx, "vxChannelCombineNode");
    platform->vxPhaseNode = (TYPE_vxPhaseNode *) GetFunctionAddress(platform->handle_openvx, "vxPhaseNode");
    platform->vxSobel3x3Node = (TYPE_vxSobel3x3Node *) GetFunctionAddress(platform->handle_openvx, "vxSobel3x3Node");
    platform->vxMagnitudeNode = (TYPE_vxMagnitudeNode *) GetFunctionAddress(platform->handle_openvx, "vxMagnitudeNode");
    platform->vxScaleImageNode = (TYPE_vxScaleImageNode *) GetFunctionAddress(platform->handle_openvx, "vxScaleImageNode");
    platform->vxTableLookupNode = (TYPE_vxTableLookupNode *) GetFunctionAddress(platform->handle_openvx, "vxTableLookupNode");
    platform->vxHistogramNode = (TYPE_vxHistogramNode *) GetFunctionAddress(platform->handle_openvx, "vxHistogramNode");
    platform->vxEqualizeHistNode = (TYPE_vxEqualizeHistNode *) GetFunctionAddress(platform->handle_openvx, "vxEqualizeHistNode");
    platform->vxAbsDiffNode = (TYPE_vxAbsDiffNode *) GetFunctionAddress(platform->handle_openvx, "vxAbsDiffNode");
    platform->vxMeanStdDevNode = (TYPE_vxMeanStdDevNode *) GetFunctionAddress(platform->handle_openvx, "vxMeanStdDevNode");
    platform->vxThresholdNode = (TYPE_vxThresholdNode *) GetFunctionAddress(platform->handle_openvx, "vxThresholdNode");
    platform->vxIntegralImageNode = (TYPE_vxIntegralImageNode *) GetFunctionAddress(platform->handle_openvx, "vxIntegralImageNode");
    platform->vxErode3x3Node = (TYPE_vxErode3x3Node *) GetFunctionAddress(platform->handle_openvx, "vxErode3x3Node");
    platform->vxDilate3x3Node = (TYPE_vxDilate3x3Node *) GetFunctionAddress(platform->handle_openvx, "vxDilate3x3Node");
    platform->vxMedian3x3Node = (TYPE_vxMedian3x3Node *) GetFunctionAddress(platform->handle_openvx, "vxMedian3x3Node");
    platform->vxBox3x3Node = (TYPE_vxBox3x3Node *) GetFunctionAddress(platform->handle_openvx, "vxBox3x3Node");
    platform->vxGaussian3x3Node = (TYPE_vxGaussian3x3Node *) GetFunctionAddress(platform->handle_openvx, "vxGaussian3x3Node");
    platform->vxNonLinearFilterNode = (TYPE_vxNonLinearFilterNode *) GetFunctionAddress(platform->handle_openvx, "vxNonLinearFilterNode");
    platform->vxConvolveNode = (TYPE_vxConvolveNode *) GetFunctionAddress(platform->handle_openvx, "vxConvolveNode");
    platform->vxGaussianPyramidNode = (TYPE_vxGaussianPyramidNode *) GetFunctionAddress(platform->handle_openvx, "vxGaussianPyramidNode");
    platform->vxLaplacianPyramidNode = (TYPE_vxLaplacianPyramidNode *) GetFunctionAddress(platform->handle_openvx, "vxLaplacianPyramidNode");
    platform->vxLaplacianReconstructNode = (TYPE_vxLaplacianReconstructNode *) GetFunctionAddress(platform->handle_openvx, "vxLaplacianReconstructNode");
    platform->vxAccumulateImageNode = (TYPE_vxAccumulateImageNode *) GetFunctionAddress(platform->handle_openvx, "vxAccumulateImageNode");
    platform->vxAccumulateWeightedImageNode = (TYPE_vxAccumulateWeightedImageNode *) GetFunctionAddress(platform->handle_openvx, "vxAccumulateWeightedImageNode");
    platform->vxAccumulateSquareImageNode = (TYPE_vxAccumulateSquareImageNode *) GetFunctionAddress(platform->handle_openvx, "vxAccumulateSquareImageNode");
    platform->vxMinMaxLocNode = (TYPE_vxMinMaxLocNode *) GetFunctionAddress(platform->handle_openvx, "vxMinMaxLocNode");
    platform->vxAndNode = (TYPE_vxAndNode *) GetFunctionAddress(platform->handle_openvx, "vxAndNode");
    platform->vxOrNode = (TYPE_vxOrNode *) GetFunctionAddress(platform->handle_openvx, "vxOrNode");
    platform->vxXorNode = (TYPE_vxXorNode *) GetFunctionAddress(platform->handle_openvx, "vxXorNode");
    platform->vxNotNode = (TYPE_vxNotNode *) GetFunctionAddress(platform->handle_openvx, "vxNotNode");
    platform->vxMultiplyNode = (TYPE_vxMultiplyNode *) GetFunctionAddress(platform->handle_openvx, "vxMultiplyNode");
    platform->vxAddNode = (TYPE_vxAddNode *) GetFunctionAddress(platform->handle_openvx, "vxAddNode");
    platform->vxSubtractNode = (TYPE_vxSubtractNode *) GetFunctionAddress(platform->handle_openvx, "vxSubtractNode");
    platform->vxConvertDepthNode = (TYPE_vxConvertDepthNode *) GetFunctionAddress(platform->handle_openvx, "vxConvertDepthNode");
    platform->vxCannyEdgeDetectorNode = (TYPE_vxCannyEdgeDetectorNode *) GetFunctionAddress(platform->handle_openvx, "vxCannyEdgeDetectorNode");
    platform->vxWarpAffineNode = (TYPE_vxWarpAffineNode *) GetFunctionAddress(platform->handle_openvx, "vxWarpAffineNode");
    platform->vxWarpPerspectiveNode = (TYPE_vxWarpPerspectiveNode *) GetFunctionAddress(platform->handle_openvx, "vxWarpPerspectiveNode");
    platform->vxHarrisCornersNode = (TYPE_vxHarrisCornersNode *) GetFunctionAddress(platform->handle_openvx, "vxHarrisCornersNode");
    platform->vxFastCornersNode = (TYPE_vxFastCornersNode *) GetFunctionAddress(platform->handle_openvx, "vxFastCornersNode");
    platform->vxOpticalFlowPyrLKNode = (TYPE_vxOpticalFlowPyrLKNode *) GetFunctionAddress(platform->handle_openvx, "vxOpticalFlowPyrLKNode");
    platform->vxRemapNode = (TYPE_vxRemapNode *) GetFunctionAddress(platform->handle_openvx, "vxRemapNode");
    platform->vxHalfScaleGaussianNode = (TYPE_vxHalfScaleGaussianNode *) GetFunctionAddress(platform->handle_openvx, "vxHalfScaleGaussianNode");
    platform->vxuColorConvert = (TYPE_vxuColorConvert *) GetFunctionAddress(platform->handle_vxu, "vxuColorConvert");
    platform->vxuChannelExtract = (TYPE_vxuChannelExtract *) GetFunctionAddress(platform->handle_vxu, "vxuChannelExtract");
    platform->vxuChannelCombine = (TYPE_vxuChannelCombine *) GetFunctionAddress(platform->handle_vxu, "vxuChannelCombine");
    platform->vxuSobel3x3 = (TYPE_vxuSobel3x3 *) GetFunctionAddress(platform->handle_vxu, "vxuSobel3x3");
    platform->vxuMagnitude = (TYPE_vxuMagnitude *) GetFunctionAddress(platform->handle_vxu, "vxuMagnitude");
    platform->vxuPhase = (TYPE_vxuPhase *) GetFunctionAddress(platform->handle_vxu, "vxuPhase");
    platform->vxuScaleImage = (TYPE_vxuScaleImage *) GetFunctionAddress(platform->handle_vxu, "vxuScaleImage");
    platform->vxuTableLookup = (TYPE_vxuTableLookup *) GetFunctionAddress(platform->handle_vxu, "vxuTableLookup");
    platform->vxuHistogram = (TYPE_vxuHistogram *) GetFunctionAddress(platform->handle_vxu, "vxuHistogram");
    platform->vxuEqualizeHist = (TYPE_vxuEqualizeHist *) GetFunctionAddress(platform->handle_vxu, "vxuEqualizeHist");
    platform->vxuAbsDiff = (TYPE_vxuAbsDiff *) GetFunctionAddress(platform->handle_vxu, "vxuAbsDiff");
    platform->vxuMeanStdDev = (TYPE_vxuMeanStdDev *) GetFunctionAddress(platform->handle_vxu, "vxuMeanStdDev");
    platform->vxuThreshold = (TYPE_vxuThreshold *) GetFunctionAddress(platform->handle_vxu, "vxuThreshold");
    platform->vxuIntegralImage = (TYPE_vxuIntegralImage *) GetFunctionAddress(platform->handle_vxu, "vxuIntegralImage");
    platform->vxuErode3x3 = (TYPE_vxuErode3x3 *) GetFunctionAddress(platform->handle_vxu, "vxuErode3x3");
    platform->vxuDilate3x3 = (TYPE_vxuDilate3x3 *) GetFunctionAddress(platform->handle_vxu, "vxuDilate3x3");
    platform->vxuMedian3x3 = (TYPE_vxuMedian3x3 *) GetFunctionAddress(platform->handle_vxu, "vxuMedian3x3");
    platform->vxuBox3x3 = (TYPE_vxuBox3x3 *) GetFunctionAddress(platform->handle_vxu, "vxuBox3x3");
    platform->vxuGaussian3x3 = (TYPE_vxuGaussian3x3 *) GetFunctionAddress(platform->handle_vxu, "vxuGaussian3x3");
    platform->vxuNonLinearFilter = (TYPE_vxuNonLinearFilter *) GetFunctionAddress(platform->handle_vxu, "vxuNonLinearFilter");
    platform->vxuConvolve = (TYPE_vxuConvolve *) GetFunctionAddress(platform->handle_vxu, "vxuConvolve");
    platform->vxuGaussianPyramid = (TYPE_vxuGaussianPyramid *) GetFunctionAddress(platform->handle_vxu, "vxuGaussianPyramid");
    platform->vxuLaplacianPyramid = (TYPE_vxuLaplacianPyramid *) GetFunctionAddress(platform->handle_vxu, "vxuLaplacianPyramid");
    platform->vxuLaplacianReconstruct = (TYPE_vxuLaplacianReconstruct *) GetFunctionAddress(platform->handle_vxu, "vxuLaplacianReconstruct");
    platform->vxuAccumulateImage = (TYPE_vxuAccumulateImage *) GetFunctionAddress(platform->handle_vxu, "vxuAccumulateImage");
    platform->vxuAccumulateWeightedImage = (TYPE_vxuAccumulateWeightedImage *) GetFunctionAddress(platform->handle_vxu, "vxuAccumulateWeightedImage");
    platform->vxuAccumulateSquareImage = (TYPE_vxuAccumulateSquareImage *) GetFunctionAddress(platform->handle_vxu, "vxuAccumulateSquareImage");
    platform->vxuMinMaxLoc = (TYPE_vxuMinMaxLoc *) GetFunctionAddress(platform->handle_vxu, "vxuMinMaxLoc");
    platform->vxuConvertDepth = (TYPE_vxuConvertDepth *) GetFunctionAddress(platform->handle_vxu, "vxuConvertDepth");
    platform->vxuCannyEdgeDetector = (TYPE_vxuCannyEdgeDetector *) GetFunctionAddress(platform->handle_vxu, "vxuCannyEdgeDetector");
    platform->vxuHalfScaleGaussian = (TYPE_vxuHalfScaleGaussian *) GetFunctionAddress(platform->handle_vxu, "vxuHalfScaleGaussian");
    platform->vxuAnd = (TYPE_vxuAnd *) GetFunctionAddress(platform->handle_vxu, "vxuAnd");
    platform->vxuOr = (TYPE_vxuOr *) GetFunctionAddress(platform->handle_vxu, "vxuOr");
    platform->vxuXor = (TYPE_vxuXor *) GetFunctionAddress(platform->handle_vxu, "vxuXor");
    platform->vxuNot = (TYPE_vxuNot *) GetFunctionAddress(platform->handle_vxu, "vxuNot");
    platform->vxuMultiply = (TYPE_vxuMultiply *) GetFunctionAddress(platform->handle_vxu, "vxuMultiply");
    platform->vxuAdd = (TYPE_vxuAdd *) GetFunctionAddress(platform->handle_vxu, "vxuAdd");
    platform->vxuSubtract = (TYPE_vxuSubtract *) GetFunctionAddress(platform->handle_vxu, "vxuSubtract");
    platform->vxuWarpAffine = (TYPE_vxuWarpAffine *) GetFunctionAddress(platform->handle_vxu, "vxuWarpAffine");
    platform->vxuWarpPerspective = (TYPE_vxuWarpPerspective *) GetFunctionAddress(platform->handle_vxu, "vxuWarpPerspective");
    platform->vxuHarrisCorners = (TYPE_vxuHarrisCorners *) GetFunctionAddress(platform->handle_vxu, "vxuHarrisCorners");
    platform->vxuFastCorners = (TYPE_vxuFastCorners *) GetFunctionAddress(platform->handle_vxu, "vxuFastCorners");
    platform->vxuOpticalFlowPyrLK = (TYPE_vxuOpticalFlowPyrLK *) GetFunctionAddress(platform->handle_vxu, "vxuOpticalFlowPyrLK");
    platform->vxuRemap = (TYPE_vxuRemap *) GetFunctionAddress(platform->handle_vxu, "vxuRemap");
    platform->vxAddKernel = (TYPE_vxAddKernel *) GetFunctionAddress(platform->handle_openvx, "vxAddKernel");
    if(!platform->vxCreateContextFromPlatform)
        return VX_FAILURE;
    return VX_SUCCESS;
}
