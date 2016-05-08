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

#ifndef _VX_INTERFACE_H_
#define _VX_INTERFACE_H_

/*!
 * \file
 * \brief The OpenVX OpenCL Target Interface
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <vx_internal.h>

#if defined(DARWIN)
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include <VX/vx_khr_opencl.h>

/*! \brief The maximum number of platforms */
#define VX_CL_MAX_PLATFORMS (1)

/*! \brief The maximum number of CL devices in the system */
#define VX_CL_MAX_DEVICES   (1)

/*! \brief The maximum number of characters on a line of OpenCL source code */
#define VX_CL_MAX_LINE_WIDTH (160)

/*! \brief The maximum path name */
#define VX_CL_MAX_PATH (256)

#ifndef VX_CL_ARGS
#define VX_CL_ARGS "-I."
#endif

#ifndef VX_CL_SOURCEPATH
#define VX_CL_SOURCEPATH ""
#endif

typedef void (*cl_notifier_f)(cl_program program, void *args);

typedef void (*cl_platform_notifier_f)(const char *errinfo,
                                       const void *private_info,
                                       size_t cb,
                                       void *user_data);

typedef struct _vx_cl_context_t {
    cl_uint          num_platforms;
    cl_uint          num_devices[VX_CL_MAX_PLATFORMS];
    cl_platform_id   platform[VX_CL_MAX_PLATFORMS];
    cl_device_id     devices[VX_CL_MAX_PLATFORMS][VX_CL_MAX_DEVICES];
    cl_context       context[VX_CL_MAX_PLATFORMS];
    cl_context_properties context_props;
    cl_command_queue queues[VX_CL_MAX_PLATFORMS][VX_CL_MAX_DEVICES];
    struct _vx_cl_kernel_description_t **kernels;
    vx_uint32 num_kernels;
} vx_cl_context_t;

#define INIT_PROGRAMS {0}
#define INIT_KERNELS  {0}
#define INIT_NUMKERNELS {0}
#define INIT_RETURNS  {0}

typedef struct _vx_cl_kernel_description_t {
    vx_kernel_description_t description;
    char             sourcepath[VX_CL_MAX_PATH];
    char             kernelname[VX_MAX_KERNEL_NAME];
    cl_program       program[VX_CL_MAX_PLATFORMS];
    cl_kernel        kernels[VX_CL_MAX_PLATFORMS];
    cl_uint          num_kernels[VX_CL_MAX_PLATFORMS];
    cl_int           returns[VX_CL_MAX_PLATFORMS][VX_CL_MAX_DEVICES];
    void            *reserved; /* for additional data */
} vx_cl_kernel_description_t;

vx_cl_kernel_description_t *vxclFindKernel(vx_enum enumeration);

extern vx_cl_kernel_description_t box3x3_clkernel;
extern vx_cl_kernel_description_t gaussian3x3_clkernel;
extern vx_cl_kernel_description_t lut_clkernel;
extern vx_cl_kernel_description_t and_kernel;
extern vx_cl_kernel_description_t xor_kernel;
extern vx_cl_kernel_description_t orr_kernel;
extern vx_cl_kernel_description_t not_kernel;
extern vx_cl_kernel_description_t histogram_kernel;

#endif


