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
#include <string.h>
#include <stdio.h>

#if _WIN32
#include <Windows.h>
#else
#include <dirent.h>
#endif

static vx_platform platform_head = NULL;

static void vxIcdAddPlatform(vx_char * fileName)
{
	size_t len = strlen(fileName);
	if(len > 0 && fileName[len-1] == '\n')
		fileName[--len] = '\0';
	if(len > 4) {
		size_t len_openvx = len;
		size_t len_vxu = 0, pos_vxu = 0;
		size_t i;
		for(i = 0; i < len; i++) {
			if(fileName[i] == ';') {
				fileName[i] = '\0';
				len_openvx = i;
				pos_vxu = i + 1;
				len_vxu = len - pos_vxu;
				break;
			}
		}
		vx_platform platform = (vx_platform)calloc(1, sizeof(struct _vx_platform));
		if(platform) {
			// initialize attributes
			platform->attributes_valid = vx_false_e;
			platform->vendor_id = VX_ID_DEFAULT;
			platform->version = VX_VERSION;
			platform->extensions_size = 0;
			platform->extensions = NULL;
			// initialize path to ICD libraries
			vx_char * path_openvx = (vx_char *)malloc(len_openvx+1);
			if(path_openvx && platform) {
				if(!platform_head) platform_head = platform;
				else {
					vx_platform platform_last = platform_head;
					while(platform_last->next)
						platform_last = platform_last->next;
					platform_last->next = platform;
				}
				platform->path_openvx = path_openvx;
				strcpy(platform->path_openvx, fileName);
				if(pos_vxu > 0) {
					platform->path_vxu = (char *)malloc(len_vxu+1);
					if(platform->path_vxu) {
						strcpy(platform->path_vxu, &fileName[pos_vxu]);
					}
				}
			}
		}
	}
}

vx_status VX_API_CALL vxIcdGetPlatforms(vx_size capacity, vx_platform platform[], vx_size * pNumItems)
{
	vx_status status = VX_SUCCESS;
	if(!platform_head) {
#if _WIN32
		HKEY key;
#if _WIN64
		LSTATUS lstatus = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Khronos\\OpenVX\\Vendors", 0, KEY_READ, &key);
#else
		LSTATUS lstatus = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Wow6432Node\\Khronos\\OpenVX\\Vendors", 0, KEY_READ, &key);
#endif
		if (lstatus == ERROR_SUCCESS) {
			vx_platform platform_last = NULL;
			DWORD index;
			for(index = 0;; index++) {
				char fileName[1024] = { 0 };
				DWORD nameType = 0, nameSize = sizeof(fileName);
				DWORD value = 0, valueSize = sizeof(value);
				if (RegEnumValueA(key, index, fileName, &nameSize, NULL, &nameType, (LPBYTE)&value, &valueSize) != ERROR_SUCCESS)
					break;
				if(nameType == REG_DWORD && value == 0) {
					vxIcdAddPlatform(fileName);
				}
			}
			RegCloseKey(key);
		}
#else
#ifdef __ANDROID__
		char *vendorPath = "/system/vendor/Khronos/OpenVX/vendors/";
#else
 		char *vendorPath = "/etc/OpenVX/vendors/";
#endif // ANDROID
		DIR * dir = opendir(vendorPath);
		if(dir) {
			vx_platform platform_last = NULL;
			struct dirent * dirEntry = readdir(dir);
			for(; dirEntry; dirEntry = readdir(dir)) {
				if(dirEntry->d_type == DT_UNKNOWN || dirEntry->d_type == DT_REG || dirEntry->d_type == DT_LNK) {
					size_t len = strlen(dirEntry->d_name);
					if(len > 4 && !strcmp(dirEntry->d_name + len - 4, ".icd")) {
						char fileName[1024];
						sprintf(fileName, "%s%s", vendorPath, dirEntry->d_name);
						FILE * fp = fopen(fileName, "r");
						if(fp) {
							if(fgets(fileName, sizeof(fileName), fp) != NULL) {
								vxIcdAddPlatform(fileName);
							}
							fclose(fp);
						}
					}
				}
			}
			closedir(dir);
		}
#endif
	}
	if(platform_head) {
		vx_size numItems = 0;
		vx_platform platform_;
		for(platform_ = platform_head; platform_ && numItems < capacity; platform_ = platform_->next) {
			platform[numItems++] = platform_;
		}
		if(pNumItems)
			*pNumItems = numItems;
	}
	return status;
}

vx_status VX_API_CALL vxQueryPlatform(vx_platform platform, vx_enum attribute, void *ptr, vx_size size)
{
	vx_status status = VX_ERROR_INVALID_REFERENCE;
	if(platform) {
		status = VX_SUCCESS;;
		if(!platform->attributes_valid) {
			// create a context from platform to initialize the platform attributes
			vx_context context = platform->vxCreateContextFromPlatform(platform);
			status = vxGetStatus((vx_reference)context);
			if(status == VX_SUCCESS) {
				status = vxReleaseContext(&context);
			}
		}
		if(platform->attributes_valid) {
			status = VX_ERROR_INVALID_PARAMETERS;
			switch(attribute) {
#ifdef VX_VERSION_1_1
			case VX_CONTEXT_VENDOR_ID:
#else
			case VX_CONTEXT_ATTRIBUTE_VENDOR_ID:
#endif
				if(size == sizeof(vx_uint16)) {
					*(vx_uint16 *)ptr = platform->vendor_id;
					status = VX_SUCCESS;
				}
				break;
#ifdef VX_VERSION_1_1
			case VX_CONTEXT_VERSION:
#else
			case VX_CONTEXT_ATTRIBUTE_VERSION:
#endif
				if(size == sizeof(vx_uint16)) {
					*(vx_uint16 *)ptr = platform->version;
					status = VX_SUCCESS;
				}
				break;
#ifdef VX_VERSION_1_1
			case VX_CONTEXT_EXTENSIONS_SIZE:
#else
			case VX_CONTEXT_ATTRIBUTE_EXTENSIONS_SIZE:
#endif
				if(size == sizeof(vx_size)) {
					*(vx_size *)ptr = platform->extensions_size;
					status = VX_SUCCESS;
				}
				break;
#ifdef VX_VERSION_1_1
			case VX_CONTEXT_EXTENSIONS:
#else
			case VX_CONTEXT_ATTRIBUTE_EXTENSIONS:
#endif
				if(size > 0 && size <= platform->extensions_size && platform->extensions) {
					strncpy(ptr, platform->extensions, size);
					status = VX_SUCCESS;
				}
				break;
			}
		}
	}
	return status;
}
