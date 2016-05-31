/*
 * Copyright (c) 2016 National Tsing Hua University.
 * \brief The FastCV OpenVX Kernel implementation
 * \author Tzu-Hsiang Lin <thlin@pllab.cs.nthu.edu.tw>
 */

#ifndef _FAST_COMMON_H_
#define _FAST_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif 

extern void vxNodeStartTimeCapture(vx_node node);
extern void vxNodeStopTimeCapture(vx_node node);

vx_status commonOneIOneO(vx_node node, vx_image src, vx_image dst, 
	fcvStatus (*fastcv_method)(const uint8_t* __restrict src, uint32_t width, uint32_t height, 
							   uint32_t srcStride, uint8_t* __restrict dst, uint32_t dstStride));

vx_status commonTwoIOneO(vx_node node, vx_image in1, vx_image in2, vx_image output,
	fcvStatus (*fastcv_method)(const uint8_t* src1, uint32_t width, uint32_t height, 
							   uint32_t src1Stride, 
							   const uint8_t* __restrict src2, uint32_t src2Stride, 
							   uint8_t* dst, uint32_t dstStride));

#ifdef __cplusplus
}
#endif 

#endif