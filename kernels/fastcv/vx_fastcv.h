/*
 * Copyright (c) 2016 National Tsing Hua University.
 * \brief The FastCV OpenVX Kernel implementation
 * \author Tzu-Hsiang Lin <thlin@pllab.cs.nthu.edu.tw>
 */

#ifndef _VX_FASTCV_H_
#define _VX_FASTCV_H_

#include <fastcv.h>
#define INIT_MEM_INFO { 0, NULL }

typedef struct mem_info
{
	vx_size len;
	void *ptr;
	vx_imagepatch_addressing_t vxImg_addr;
} mem_info_t;

vx_status map_vx_image_for_read(vx_image vxImg, uint32_t *width, uint32_t *height, mem_info_t *mem_info);

vx_status map_vx_image_for_write(vx_image vxImg, mem_info_t *mem_info);

vx_status unmap_vx_image(vx_image vxImg, mem_info_t *mem_info);

#endif