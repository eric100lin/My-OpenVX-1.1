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

#include <stdio.h>
#include <vx.hpp>

using namespace OpenVX;

void list_kernels(Context* pContext)
{
#if defined(EXPERIMENTAL_USE_TARGET)
    vx_uint32 numTargets = pContext->numTargets();
#endif
    vx_uint32 numModules = pContext->numModules();
    vx_uint32 numKernels = pContext->numKernels();

    printf("Implementation: %s\n", pContext->implementation());
    printf("Vendor: %hu\n", pContext->vendorID());
    printf("Version: %04x\n", pContext->version());
    printf("Extensions: %s\n", pContext->extensions());
    printf("Num Modules = %u\n", numModules);
#if defined(EXPERIMENTAL_USE_TARGET)
    printf("Num Targets = %u\n", numTargets);
#endif
    printf("Num Kernels = %u\n", numKernels);

#if defined(EXPERIMENTAL_USE_TARGET)
    for (vx_uint32 t = 0; t < numTargets; t++)
    {
        Target* pTarget = pContext->createTarget(t);
        vx_kernel_info_t *table = pTarget->kernelTable();
        printf("Target %s\n", pTarget->name());
        for (vx_uint32 k = 0; table && k < pTarget->numKernels(); k++)
        {
//            Node* pNode = pContext->createNode(table[k].enumeration); // or by name
//            printf("enum %08x == %s params:%u\n", table[k].enumeration, table[k].name, pKernel->numParameters());
        }
    }
#else
    /*! \todo implement C++ kernel list from context */
#endif
}

int main(int argc, char *argv[])
{
    vx_status status = VX_FAILURE;

    Context context;
    Graph* pGraph = context.createGraph();

#define TEST 1
#if (TEST==1)

    String* pFilenameIn  = context.createString("lena_512x512.pgm");
    String* pFilenameOut = context.createString("ocpplena.pgm");
    Image*  pInput       = context.createImage(512, 512, VX_DF_IMAGE_U8);

    FReadImageNode  fr(pGraph, pInput, pFilenameIn);
    FWriteImageNode fw(pGraph, pInput, pFilenameOut);

#else

    String* pFilenameIn  = context.createString("lena_512x512.pgm");
    String* pFilenameOut = context.createString("ocpplena.pgm");
    Image*  pInput       = context.createImage(512, 512, VX_DF_IMAGE_U8);
    Image*  pGx          = context.createImage();
    Image*  pGy          = context.createImage();
    Image*  pOutput      = context.createImage(VX_DF_IMAGE_U8);

    FReadImageNode  fr   (pGraph, pInput, pFilenameIn);
    Sobel3x3Node    sobel(pGraph, pInput, pGx, pGy);
    MagnitudeNode   mag  (pGraph, pGx, pGy, pOutput);
    FWriteImageNode fw   (pGraph, pOutput, pFilenameOut);

#endif

    status = pGraph->verify();
    if (status == VX_SUCCESS)
    {
        status = pGraph->process();
        if (status == VX_SUCCESS)
        {
            printf("Ran graph!\n");
        }
    }
    printf("Returning status %d\n", status);
    return status;
}

