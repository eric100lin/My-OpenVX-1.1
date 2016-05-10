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

#include <stdlib.h>
#include <vx.hpp>

using namespace OpenVX;

Context::Context()
{
    m_context = vxCreateContext();
    m_extensions = NULL;
}

Context::~Context()
{
    vxReleaseContext(&m_context);
}

vx_status Context::setImmediateBorderMode(vx_border_t *config)
{
    return vxSetContextAttribute(context(), VX_CONTEXT_IMMEDIATE_BORDER, config, sizeof(*config));
}

vx_status Context::hint(vx_reference reference, vx_enum hint, const void* data, vx_size data_size)
{
    return vxHint(reference, hint, data, data_size);
}

vx_status Context::directive(vx_reference reference, vx_enum directive)
{
    return vxDirective(reference, directive);
}

vx_status Context::loadKernels(const char *filepath)
{
    return vxLoadKernels(context(), (vx_char *)filepath);
}

// vxQueryContext

vx_uint16 Context::vendorID()
{
    vx_uint16 vendor_id;
    vxQueryContext(context(), VX_CONTEXT_VENDOR_ID, &vendor_id, sizeof(vendor_id));
    return vendor_id;
}

vx_uint16 Context::version()
{
    vx_uint16 version;
    vxQueryContext(context(), VX_CONTEXT_VERSION, &version, sizeof(version));
    return version;
}

vx_uint32 Context::numKernels()
{
    vx_uint32 kernels = 0;
    vxQueryContext(context(), VX_CONTEXT_UNIQUE_KERNELS, &kernels, sizeof(kernels));
    return kernels;
}

vx_uint32 Context::numModules()
{
    vx_uint32 modules = 0;
    vxQueryContext(context(), VX_CONTEXT_MODULES, &modules, sizeof(modules));
    return modules;
}

vx_uint32 Context::numRefs()
{
    vx_uint32 refs = 0;
    vxQueryContext(context(), VX_CONTEXT_REFERENCES, &refs, sizeof(refs));
    return refs;
}

#if defined(EXPERIMENTAL_USE_TARGET)
vx_uint32 Context::numTargets()
{
    vx_uint32 targets = 0;
    vxQueryContext(context(), VX_CONTEXT_TARGETS, &targets, sizeof(targets));
    return targets;
}
#endif

const char* Context::implementation()
{
    vxQueryContext(context(), VX_CONTEXT_IMPLEMENTATION, m_implementationName, sizeof(m_implementationName));
    return m_implementationName;
}

vx_size Context::extensionSize()
{
    vx_size size = 0;
    vxQueryContext(context(), VX_CONTEXT_EXTENSIONS_SIZE, &size, sizeof(size));
    return size;
}

const char *Context::extensions()
{
    if (m_extensions == NULL)
    {
        vx_size extsize = extensionSize();
        m_extensions = (vx_char *)calloc(sizeof(vx_char), extsize);
        if (m_extensions)
        {
            vxQueryContext(context(), VX_CONTEXT_EXTENSIONS, m_extensions, extsize);
        }
    }
    return m_extensions;
}

vx_size Context::convolutionMaxDimension()
{
    vx_size size = 0;
    vxQueryContext(context(), VX_CONTEXT_CONVOLUTION_MAX_DIMENSION, &size, sizeof(size));
    return size;
}

// object factory functions
// for reasons unknown, these cannot be put inline with all the rest without creating compiler errors

Delay<Image>* Context::createDelay(vx_uint32 width, vx_uint32 height, vx_df_image format, vx_size num)
{
    return (new Delay<Image>(this, width, height, format, num));
}

