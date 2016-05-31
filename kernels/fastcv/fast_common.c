/*
 * Copyright (c) 2016 National Tsing Hua University.
 * \brief The FastCV OpenVX Kernel implementation
 * \author Tzu-Hsiang Lin <thlin@pllab.cs.nthu.edu.tw>
 */

#include "fastcv_model.h"
#include "vx_fastcv.h"
#include "fast_common.h"
#include <vx_debug.h>

vx_status commonOneIOneO(vx_node node, vx_image src, vx_image dst, 
	fcvStatus (*fastcv_method)(const uint8_t* __restrict src, uint32_t width, uint32_t height,
							   uint32_t srcStride, uint8_t* __restrict dst, uint32_t dstStride))
{
	vx_status status = VX_SUCCESS;
	mem_info_t src_mem_info = INIT_MEM_INFO;
	mem_info_t dst_mem_info = INIT_MEM_INFO;
	fcvStatus fastcv_ret;
	uint32_t width, height, srcStride, dstStride;
	
	status = map_vx_image_for_read(src, &width, &height, &src_mem_info);
	if(status != VX_SUCCESS)	goto error_read_image;
	
	status = map_vx_image_for_write(dst, &dst_mem_info);
	if(status != VX_SUCCESS)	goto error_allocate_dst_image;

	srcStride = width;	dstStride = width;
	vxNodeStartTimeCapture(node);				//Measure DSP computing time START
	fastcv_ret = fastcv_method((uint8_t *)src_mem_info.ptr, 
							   width, height, srcStride, 
							   (uint8_t *)dst_mem_info.ptr,
							   dstStride);
	vxNodeStopTimeCapture(node);				//Measure DSP computing time STOP

	if(fastcv_ret != 0) 
	{
		status = VX_FAILURE;
		goto error_processing;
	}
	
error_processing:
	unmap_vx_image(dst, &dst_mem_info);
error_allocate_dst_image:
	unmap_vx_image(src, &src_mem_info);
error_read_image:
	return status;
}

vx_status commonTwoIOneO(vx_node node, vx_image in1, vx_image in2, vx_image output,
	fcvStatus (*fastcv_method)(const uint8_t* src1, uint32_t width, uint32_t height, 
							   uint32_t src1Stride, 
							   const uint8_t* __restrict src2, uint32_t src2Stride, 
							   uint8_t* dst, uint32_t dstStride))
{
	vx_status status = VX_SUCCESS;
	mem_info_t src1_mem_info = INIT_MEM_INFO;
	mem_info_t src2_mem_info = INIT_MEM_INFO;
	mem_info_t dst_mem_info = INIT_MEM_INFO;
	fcvStatus fastcv_ret;
	uint32_t width, height, srcStride, dstStride;
	
	status = map_vx_image_for_read(in1, &width, &height, &src1_mem_info);
	if(status != VX_SUCCESS)	goto error_read_image1;
	
	status = map_vx_image_for_read(in2, &width, &height, &src2_mem_info);
	if(status != VX_SUCCESS)	goto error_read_image2;
	
	status = map_vx_image_for_write(output, &dst_mem_info);
	if(status != VX_SUCCESS)	goto error_allocate_dst_image;
	
	srcStride = width;	dstStride = width;
	vxNodeStartTimeCapture(node);				//Measure DSP computing time START
	fastcv_ret = fastcv_method((uint8_t *)src1_mem_info.ptr, 
							   width, height, srcStride, 
							   (uint8_t *)src2_mem_info.ptr, 
							   srcStride, 
							   (uint8_t *)dst_mem_info.ptr, 
							   dstStride);
	vxNodeStopTimeCapture(node);				//Measure DSP computing time STOP
	if(fastcv_ret != 0) 
	{
		status = VX_FAILURE;
		goto error_processing;
	}

error_processing:
	unmap_vx_image(output, &dst_mem_info);
error_allocate_dst_image:
	unmap_vx_image(in2, &src2_mem_info);
error_read_image2:
	unmap_vx_image(in1, &src1_mem_info);
error_read_image1:
	return status;
}
