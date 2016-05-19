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

#include <VX/vx.h>
#include <vx_debug.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(_WIN32)
#include <Windows.h>
#endif

#ifndef dimof
#define dimof(x) (sizeof(x)/sizeof(x[0]))
#endif

void vx_print(vx_enum zone, char *format, ...);

static vx_uint32 vx_zone_mask;

#undef  ZONE_BIT
#define ZONE_BIT(zone)  (1 << zone)

void vx_set_debug_zone(vx_enum zone)
{
    if (0 <= zone && zone < VX_ZONE_MAX) {
        vx_zone_mask |= ZONE_BIT(zone);
        vx_print(zone, "Enabled\n");
        //printf("vx_debug_mask=0x%08x\n", vx_debug_mask);
    }
}

void vx_clr_debug_zone(vx_enum zone)
{
    if (0 <= zone && zone < VX_ZONE_MAX) {
        vx_print(zone, "Disabled\n");
        vx_zone_mask &= ~(ZONE_BIT(zone));
        //printf("vx_debug_mask=0x%08x\n", vx_debug_mask);
    }
}

vx_bool vx_get_debug_zone(vx_enum zone)
{
    if (0 <= zone && zone < VX_ZONE_MAX)
        return ((vx_zone_mask & zone)?vx_true_e:vx_false_e);
    else
        return vx_false_e;
}

void vx_set_debug_zone_from_env(void)
{
    char *str = getenv("VX_ZONE_MASK");
    if (str)
    {
        sscanf(str, "%x", &vx_zone_mask);
    }
    else
    {
        str = getenv("VX_ZONE_LIST");
        if (str)
        {
            vx_char *num = NULL;
#if defined(_WIN32)
            char *buf = _strdup(str);
#else
            char *buf = strdup(str);
#endif
            str = buf;
            do {
                vx_enum zone = -1;
                num = strtok(str, ",");
                str = NULL;
                if (num)
                {
                    //printf("num=%s\n", num);
                    zone = atoi(num);
                    vx_set_debug_zone(zone);
                }
            } while (num != NULL);
            free(buf);
        }
    }
    //printf("vx_zone_mask = 0x%08x\n", vx_zone_mask);
}

#define _STR2(x) {#x, x}

struct vx_string_and_enum_e {
    vx_char name[20];
    vx_enum value;
};

struct vx_string_and_enum_e enumnames[] = {
    _STR2(VX_ZONE_ERROR),
    _STR2(VX_ZONE_WARNING),
    _STR2(VX_ZONE_API),
    _STR2(VX_ZONE_INFO),
    _STR2(VX_ZONE_PERF),
    _STR2(VX_ZONE_CONTEXT),
    _STR2(VX_ZONE_OSAL),
    _STR2(VX_ZONE_REFERENCE),
    _STR2(VX_ZONE_ARRAY),
    _STR2(VX_ZONE_IMAGE),
    _STR2(VX_ZONE_SCALAR),
    _STR2(VX_ZONE_KERNEL),
    _STR2(VX_ZONE_GRAPH),
    _STR2(VX_ZONE_NODE),
    _STR2(VX_ZONE_PARAMETER),
    _STR2(VX_ZONE_DELAY),
    _STR2(VX_ZONE_TARGET),
    _STR2(VX_ZONE_LOG),
	_STR2(VX_ZONE_TAR_HEXAGON),
    {"UNKNOWN", -1}, // if the zone is not found, this will be returned.
};

vx_char *find_zone_name(vx_enum zone)
{
    vx_uint32 i;
    for (i = 0; i < dimof(enumnames); i++)
    {
        if (enumnames[i].value == zone)
        {
            break;
        }
    }
    return enumnames[i].name;
}

#if defined(_WIN32) && !defined(__CYGWIN__)

void vx_print(vx_enum zone, char *string, ...)
{
    if (vx_zone_mask & ZONE_BIT(zone))
    {
        char format[1024];
        va_list ap;
        _snprintf(format, sizeof(format), "%20s: %08x: %s", find_zone_name(zone), GetCurrentThreadId(), string);
        format[sizeof(format)-1] = 0; // for MSVC which is not C99 compliant
        va_start(ap, string);
        vprintf(format, ap);
        va_end(ap);
    }
}

#elif defined(__ANDROID__)

#include <android/log.h>

void vx_print(vx_enum zone, char *format, ...)
{
    if (vx_zone_mask & ZONE_BIT(zone))
    {
        char string[1024];
        va_list ap;
        snprintf(string, sizeof(string), "%20s:%s", find_zone_name(zone), format);
        va_start(ap, format);
        __android_log_vprint(ANDROID_LOG_DEBUG, "OpenVX", string, ap);
        va_end(ap);
    }
}

#elif defined(__linux__) || defined(__QNX__) || defined(__APPLE__) || defined(__CYGWIN__)

void vx_print(vx_enum zone, char *format, ...)
{
    if (vx_zone_mask & ZONE_BIT(zone))
    {
        char string[1024];
        va_list ap;
        snprintf(string, sizeof(string), "%20s:%s", find_zone_name(zone), format);
        va_start(ap, format);
        vprintf(string, ap);
        va_end(ap);
    }
}

#endif

void vxPrintImageAddressing(const vx_imagepatch_addressing_t *addr)
{
    if (addr)
    {
        VX_PRINT(VX_ZONE_IMAGE, "addr:%p dim={%u,%u} stride={%d,%d} scale={%u,%u} step={%u,%u}\n",
                addr,
                addr->dim_x, addr->dim_y,
                addr->stride_x, addr->stride_y,
                addr->scale_x, addr->scale_y,
                addr->step_x, addr->step_y);
    }
}


