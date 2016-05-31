/*
 * Copyright (c) 2016 National Tsing Hua University.
 * \brief The FastCV OpenVX Kernel implementation
 * \author Tzu-Hsiang Lin <thlin@pllab.cs.nthu.edu.tw>
 */

#include "fastcv_model.h"
#include "vx_fastcv.h"
#include <vx_debug.h>

vx_status vxFastCVInit()
{
	char sVersion[32];  
	fcvSetOperationMode( (fcvOperationMode) FASTCV_OP_CPU_PERFORMANCE );

	fcvGetVersion(sVersion, 32);
	
	VX_PRINT(VX_ZONE_INFO, "Using FastCV version version %s\n", sVersion);
	
	return VX_SUCCESS;
}

vx_status vxFastCVDeInit()
{
	fcvCleanUp();
	return VX_SUCCESS;
}

static vx_status get_vx_image_size(vx_image vxImg, vx_size *vxImgSize)
{
	vx_status status = VX_SUCCESS;
	vx_rectangle_t rect;
	vx_size n_plane, vxImgPlanes = 0ul;

	status = vxGetValidRegionImage(vxImg, &rect);
	if(status != VX_SUCCESS) return status;
	
	status = vxQueryImage(vxImg, VX_IMAGE_PLANES, &vxImgPlanes, sizeof(vxImgPlanes));
	if(status != VX_SUCCESS) return status;
	
	*vxImgSize = 0ul;
	for(n_plane=0; n_plane<vxImgPlanes; n_plane++)
		(*vxImgSize) += vxComputeImagePatchSize(vxImg, &rect, n_plane);
	if(*vxImgSize == 0)	return VX_ERROR_INVALID_VALUE;
	
	VX_PRINT(VX_ZONE_TAR_HEXAGON, "vxImgSize %d\n", *vxImgSize);
	return status;
}

static vx_status map_vx_image(vx_image vxImg, uint32_t *width, uint32_t *height, mem_info_t *mem_info, vx_enum usage)
{
	vx_size vxImgSize;
	vx_status status = VX_SUCCESS;
	vx_rectangle_t rect;
	vx_imagepatch_addressing_t vxImg_addr;
	void *vxImg_base = NULL;
	
	status = vxGetValidRegionImage(vxImg, &rect);
	if(status != VX_SUCCESS) return status;
	
	status = vxAccessImagePatch(vxImg, &rect, 0, &vxImg_addr, (void **)&vxImg_base, usage);
	if(status != VX_SUCCESS) return status;
	
    *width  = vxImg_addr.dim_x;
    *height = vxImg_addr.dim_y;
	
	status = get_vx_image_size(vxImg, &vxImgSize);
	if(status != VX_SUCCESS) return status;
	
	mem_info->len = vxImgSize;
	mem_info->ptr = vxImg_base;
	mem_info->vxImg_addr = vxImg_addr;
	
	return status;
}

vx_status map_vx_image_for_read(vx_image vxImg, uint32_t *width, uint32_t *height, mem_info_t *mem_info)
{
	vx_status status = map_vx_image(vxImg, width, height, mem_info, VX_READ_ONLY);
	VX_PRINT(VX_ZONE_INFO, "map_vx_image_for_read returned %d\n", status);
	return status;
}

vx_status map_vx_image_for_write(vx_image vxImg, mem_info_t *mem_info)
{
	uint32_t width, height;
	vx_status status = map_vx_image(vxImg, &width, &height, mem_info, VX_WRITE_ONLY);
	VX_PRINT(VX_ZONE_INFO, "map_vx_image_for_write returned %d\n", status);
	return status;
}

vx_status unmap_vx_image(vx_image vxImg, mem_info_t *mem_info)
{
	vx_status status = VX_SUCCESS;
	vx_rectangle_t rect;
	
	if (!mem_info->ptr) 
		return VX_ERROR_INVALID_VALUE;
	
	status = vxGetValidRegionImage(vxImg, &rect);
	if(status != VX_SUCCESS) return status;
	
	vxCommitImagePatch(vxImg, &rect, 0, &mem_info->vxImg_addr, mem_info->ptr);
	
	VX_PRINT(VX_ZONE_INFO, "unmap_vx_image len:%d ptr:%x\n", mem_info->len, mem_info->ptr);
}