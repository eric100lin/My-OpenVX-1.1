# This script will generate vx_dispatch.h, vx_dispatch.c, and
# ../include/VX/vx_khr_icd.h for use by vx_khr_icd extension
# Run the below command from vx_khr_icd folder:
#  % python vx_khr_icd.py ../include/VX/vx_api.h ../include/VX/vx_nodes.h ../include/VX/vxu.h ../include/VX/vx_compatibility.h
# Note that ../include/VX/vx_compatibility.h is optional
#
# Author: Radhakrishna Giduthuri (radha.giduthuri@amd.com)

import fileinput
import sys
import os

if len(sys.argv) < 3:
	print('This script will generate vx_dispatch.h, vx_dispatch.c, and')
	print('../include/VX/vx_khr_icd.h for use by vx_khr_icd extension')
	print('Run the below command from vx_khr_icd folder:')
	print(' % python vx_khr_icd.py ../include/VX/vx_api.h ../include/VX/vx_nodes.h ../include/VX/vxu.h ../include/VX/vx_compatibility.h')
	print('Note that ../include/VX/vx_compatibility.h is optional')
	sys.exit(1)

copyright = """/*
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
"""

includeCompatibility = False
for arg in sys.argv:
	if 'VX/vx_compatibility.h' in arg:
		includeCompatibility = True
		break

sourceVersionIs11OrLater = True
fdecl = ['vx_context VX_API_CALL vxCreateContextFromPlatform(vx_platform platform);']
decl = ''
for line in fileinput.input():
	if decl == '' and line[:13] == 'VX_API_ENTRY ':
		decl = line[:-1]
	elif decl != '':
		decl = decl + line[:-1]
	if ');' in decl:
		line = ' '
		for c in decl:
			if not (c in ' \t\n') or (line[-1] != ' '):
				line = line + c
		decl = line[14:].strip()
		if not ('vxFormatImagePatchAddress' in decl):
			fdecl.append(decl)
			if 'vxHint' in decl:
				if 'vx_enum hint);' in decl:
					sourceVersionIs11OrLater = False
				else:
					fdecl.append('vx_status VX_API_CALL vxHint101(vx_reference reference, vx_enum hint);')
		decl = ''

print('OK: creating vx_dispatch.h ...')
fp = open('vx_dispatch.h', 'w')
fp.write(copyright + """
#ifndef _VX_DISPATCH_H_
#define _VX_DISPATCH_H_

#include <VX/vx_khr_icd.h>
""")
if includeCompatibility:
	fp.write('#include <VX/vx_compatibility.h>\n')
fp.write('\n')
for decl in fdecl:
	pos = decl.find('VX_API_CALL')
	fp.write('typedef ' + decl[:pos+12] + 'TYPE_' + decl[pos+12:] + '\n')
fp.write('\n')
fp.write('struct _vx_platform {\n')
fp.write('    vx_platform   next;\n')
fp.write('    vx_char     * path_openvx;\n')
fp.write('    vx_char     * path_vxu;\n')
fp.write('    vx_bool       attributes_valid;\n')
fp.write('    vx_uint16     vendor_id;\n')
fp.write('    vx_uint16     version;\n')
fp.write('    vx_size       extensions_size;\n')
fp.write('    vx_char     * extensions;\n')
fp.write('    void        * handle_openvx;\n')
fp.write('    void        * handle_vxu;\n')
for decl in fdecl:
	pos = decl.find('VX_API_CALL')
	name = decl[pos+12:pos+12+decl[pos+12:].find('(')]
	fp.write('    TYPE_' + name + ' * ' + name + ';\n')
fp.write('};\n')
fp.write('\n')
fp.write('#endif\n')
fp.close()

print('OK: creating vx_dispatch.c ...')
fp = open('vx_dispatch.c', 'w')
fp.write(copyright)
fp.write('\n')
fp.write('#define _CRT_SECURE_NO_WARNINGS\n')
fp.write('#include "vx_dispatch.h"\n')
fp.write('#include <stdlib.h>\n')
fp.write('#include <stdarg.h>\n')
fp.write('#include <stdio.h>\n')
fp.write('\n')
fp.write('#if _WIN32\n')
fp.write('#include <Windows.h>\n')
fp.write('#define OpenSharedLibrary(libraryPath)      LoadLibraryA(libraryPath)\n')
fp.write('#define GetFunctionAddress(handle,funcName) GetProcAddress(handle,funcName)\n')
fp.write('#else\n')
fp.write('#include <dlfcn.h>\n')
fp.write('#define OpenSharedLibrary(libraryPath)      dlopen(libraryPath, RTLD_NOW|RTLD_LOCAL)\n')
fp.write('#define GetFunctionAddress(handle,funcName) dlsym(handle,funcName)\n')
fp.write('#endif\n')
fp.write('\n')
fp.write('#define CHECK_OBJECT(obj,retValue)                 if(!obj) return retValue\n')
fp.write('#define CHECK_OBJECT_VOID(obj)                     if(!obj) return\n')
fp.write('#define CHECK_OBJECT_PTR(objPtr,retValue)          if(!objPtr || !*objPtr) return retValue\n')
fp.write('#define CHECK_DISPATCH(platform,funcName,retValue) if(!platform || !platform->funcName) return retValue\n')
fp.write('#define CHECK_DISPATCH_VOID(platform,funcName)     if(!platform || !platform->funcName) return\n')
fp.write('\n')
fp.write('static vx_status VX_API_ENTRY vxLoadDispatchTable(vx_platform platform);\n')
for decl in fdecl:
	pos = decl.find('VX_API_CALL')
	name = decl[pos+12:pos+12+decl[pos+12:].find('(')]
	if name == 'vxHint101':
		continue
	fp.write('\n')
	fp.write(decl[:-1] + '\n')
	fp.write('{\n')
	paramName = []
	paramIsPtr = []
	for arg in decl[decl.find('(')+1:-2].split(','):
		if len(arg) > 0:
			isPtr = False
			arg = arg.split(' ')[-1]
			while arg[0] == '*':
				arg = arg[1:]
				isPtr = True
			if '[' in arg:
				arg = arg[:arg.find('[')]
			paramName.append(arg)
			paramIsPtr.append(isPtr)
	if name == 'vxCreateContext':
		fp.write('    vx_context context = NULL;\n')
		fp.write('    vx_platform platform = NULL;\n')
		fp.write('    vx_status status = vxIcdGetPlatforms(1, &platform, NULL);\n')
		fp.write('    if(status == VX_SUCCESS)\n')
		fp.write('        context = vxCreateContextFromPlatform(platform);\n')
		fp.write('    return context;\n')
	elif name == 'vxCreateContextFromPlatform':
		fp.write('    if(platform && !platform->handle_openvx) {\n')
		fp.write('        vx_status status = vxLoadDispatchTable(platform);\n')
		fp.write('        if(status != VX_SUCCESS)\n')
		fp.write('            return NULL;\n')
		fp.write('    }\n')
		fp.write('    CHECK_DISPATCH(platform, vxCreateContextFromPlatform, NULL);\n')
		fp.write('    vx_context context = platform->vxCreateContextFromPlatform(platform);\n')
		fp.write('    if(!platform->attributes_valid && vxGetStatus((vx_reference)context) == VX_SUCCESS) {\n')
		fp.write('        platform->attributes_valid = vx_true_e;\n')
		if sourceVersionIs11OrLater:
			fp.write('        vxQueryContext(context, VX_CONTEXT_VENDOR_ID, &platform->vendor_id, sizeof(platform->vendor_id));\n')
			fp.write('        vxQueryContext(context, VX_CONTEXT_VERSION, &platform->version, sizeof(platform->version));\n')
			fp.write('        vxQueryContext(context, VX_CONTEXT_EXTENSIONS_SIZE, &platform->extensions_size, sizeof(platform->extensions_size));\n')
		else:
			fp.write('        vxQueryContext(context, VX_CONTEXT_ATTRIBUTE_VENDOR_ID, &platform->vendor_id, sizeof(platform->vendor_id));\n')
			fp.write('        vxQueryContext(context, VX_CONTEXT_ATTRIBUTE_VERSION, &platform->version, sizeof(platform->version));\n')
			fp.write('        vxQueryContext(context, VX_CONTEXT_ATTRIBUTE_EXTENSIONS_SIZE, &platform->extensions_size, sizeof(platform->extensions_size));\n')
		fp.write('        if(platform->extensions_size > 0) {\n')
		fp.write('            platform->extensions = (vx_char *)calloc(1, platform->extensions_size + 1);\n')
		fp.write('            if(!platform->extensions) {\n')
		fp.write('                platform->extensions_size = 0;\n')
		fp.write('            }\n')
		fp.write('            else {\n')
		if sourceVersionIs11OrLater:
			fp.write('                vxQueryContext(context, VX_CONTEXT_EXTENSIONS, platform->extensions, platform->extensions_size);\n')
		else:
			fp.write('                vxQueryContext(context, VX_CONTEXT_ATTRIBUTE_EXTENSIONS, platform->extensions, platform->extensions_size);\n')
		fp.write('            }\n')
		fp.write('        }\n')
		if sourceVersionIs11OrLater:
			fp.write('        if(platform->version == VX_VERSION_1_0) {\n')
			fp.write('            platform->vxHint101 = (TYPE_vxHint101 *)platform->vxHint;\n')
			fp.write('            platform->vxHint = NULL;\n')
			fp.write('        }\n')
		fp.write('    }\n')
		fp.write('    return context;\n')
	elif (name == 'vxHint') and (len(paramName) == 4):
		fp.write('    CHECK_OBJECT(' + paramName[0] + ', VX_ERROR_INVALID_REFERENCE);\n')
		fp.write('    vx_platform platform = *(vx_platform *) ' + paramName[0] + ';\n')
		fp.write('    CHECK_OBJECT(platform, VX_ERROR_NOT_IMPLEMENTED);\n')
		fp.write('    if(platform->vxHint)\n')
		fp.write('        return platform->vxHint(%s, %s, %s, %s);\n' % (paramName[0], paramName[1], paramName[2], paramName[3]))
		fp.write('    else if(platform->vxHint101)\n')
		fp.write('        return platform->vxHint101(%s, %s);\n' % (paramName[0], paramName[1]))
		fp.write('    else\n')
		fp.write('        return VX_ERROR_NOT_IMPLEMENTED;\n')
	else:
		if paramIsPtr[0]:
			fp.write('    CHECK_OBJECT_PTR(' + paramName[0] + ', VX_ERROR_INVALID_REFERENCE);\n')
			fp.write('    vx_platform platform = *(vx_platform *) *' + paramName[0] + ';\n')
		else:
			if decl[:9] == 'vx_status':
				fp.write('    CHECK_OBJECT(' + paramName[0] + ', VX_ERROR_INVALID_REFERENCE);\n')
			elif decl[:7] == 'vx_enum':
				fp.write('    CHECK_OBJECT(' + paramName[0] + ', VX_TYPE_INVALID);\n')
			elif decl[:7] == 'vx_size':
				fp.write('    CHECK_OBJECT(' + paramName[0] + ', 0);\n')
			elif decl[:7] == 'vx_bool':
				fp.write('    CHECK_OBJECT(' + paramName[0] + ', vx_false_e);\n')
			elif decl[:4] == 'void':
				fp.write('    CHECK_OBJECT_VOID(' + paramName[0] + ');\n')
			else:
				fp.write('    CHECK_OBJECT(' + paramName[0] + ', NULL);\n')
			if paramName[0] != 'platform':
				fp.write('    vx_platform platform = *(vx_platform *) ' + paramName[0] + ';\n')
		if decl[:9] == 'vx_status':
			fp.write('    CHECK_DISPATCH(platform, ' + name + ', VX_ERROR_NOT_IMPLEMENTED);\n')
		elif decl[:7] == 'vx_enum':
			fp.write('    CHECK_DISPATCH(platform, ' + name + ', VX_TYPE_INVALID);\n')
		elif decl[:7] == 'vx_size':
			fp.write('    CHECK_DISPATCH(platform, ' + name + ', 0);\n')
		elif decl[:7] == 'vx_bool':
			fp.write('    CHECK_DISPATCH(platform, ' + name + ', vx_false_e);\n')
		elif decl[:4] == 'void':
			fp.write('    CHECK_DISPATCH_VOID(platform, ' + name + ');\n')
		else:
			fp.write('    CHECK_DISPATCH(platform, ' + name + ', NULL);\n')
		if paramName[-1] == '...':
			fp.write('    va_list ap;\n')
			fp.write('    vx_char _message[VX_MAX_LOG_MESSAGE_LEN];\n')
			fp.write('    va_start(ap, ' + paramName[-2] + ');\n')
			fp.write('    vsnprintf(_message, VX_MAX_LOG_MESSAGE_LEN, ' + paramName[-2] + ', ap);\n')
			fp.write('    _message[VX_MAX_LOG_MESSAGE_LEN - 1] = 0; // for MSVC which is not C99 compliant\n')
			fp.write('    va_end(ap);\n')
			if decl[:4] == 'void':
				line = '    platform->' + name + '(' + paramName[0]
			else:
				line = '    return platform->' + name + '(' + paramName[0]
			for item in paramName[1:-2]:
				line = line + ', ' + item
			line = line + ', _message);\n'
			fp.write(line)
		else:
			if decl[:4] == 'void':
				line = '    platform->' + name + '(' + paramName[0]
			else:
				line = '    return platform->' + name + '(' + paramName[0]
			for item in paramName[1:]:
				line = line + ', ' + item
			line = line + ');\n'
			fp.write(line)
	fp.write('}\n')
fp.write("""
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
""")
fp.write('\n')
fp.write('static vx_status VX_API_ENTRY vxLoadDispatchTable(vx_platform platform)\n')
fp.write('{\n')
fp.write('    platform->handle_openvx = OpenSharedLibrary(platform->path_openvx);\n')
fp.write('    if(!platform->handle_openvx)\n')
fp.write('        return VX_FAILURE;\n')
fp.write('    if(platform->path_vxu && strcmp(platform->path_vxu, platform->path_openvx) != 0) {\n')
fp.write('        platform->handle_vxu = OpenSharedLibrary(platform->path_vxu);\n')
fp.write('        if(!platform->handle_vxu)\n')
fp.write('            return VX_FAILURE;\n')
fp.write('    }\n')
fp.write('    else {\n')
fp.write('        platform->handle_vxu = platform->handle_openvx;\n')
fp.write('    }\n')
for decl in fdecl:
	pos = decl.find('VX_API_CALL')
	name = decl[pos+12:pos+12+decl[pos+12:].find('(')]
	if name == 'vxHint101':
		continue
	if name[:3] == 'vxu':
		fp.write('    platform->' + name + ' = (TYPE_' + name + ' *) GetFunctionAddress(platform->handle_vxu, "' + name + '");\n')
	else:
		fp.write('    platform->' + name + ' = (TYPE_' + name + ' *) GetFunctionAddress(platform->handle_openvx, "' + name + '");\n')
fp.write('    if(!platform->vxCreateContextFromPlatform)\n')
fp.write('        return VX_FAILURE;\n')
fp.write('    return VX_SUCCESS;\n')
fp.write('}\n')
fp.close()

if os.path.isdir('../include/VX'):
	print('OK: creating ../include/VX/vx_khr_icd.h ...')
	fi = open('vx_khr_icd.h', 'r')
	fo = open('../include/VX/vx_khr_icd.h', 'w')
	fo.write(fi.read())
	fo.close()
	fi.close()
