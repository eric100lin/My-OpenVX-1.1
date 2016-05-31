/*
 * Copyright (c) 2016 National Tsing Hua University.
 * \brief The FastCV OpenVX Kernel implementation
 * \author Tzu-Hsiang Lin <thlin@pllab.cs.nthu.edu.tw>
 */

#include "fastcv_model.h"
#include "vx_fastcv.h"
#include "fast_common.h"

vx_status vxMedian3x3(vx_node node, vx_image src, vx_image dst, vx_border_t *borders)
{
	return VX_ERROR_NOT_IMPLEMENTED;
}

static fcvStatus wrap_fcvBoxFilter3x3u8( const uint8_t* __restrict src,
                           unsigned int              srcWidth,
                           unsigned int              srcHeight,
                           unsigned int              srcStride,
                           uint8_t* __restrict       dst,
                           unsigned int              dstStride)
{
	fcvBoxFilter3x3u8(src, srcWidth, srcHeight, srcStride, dst, dstStride);
	return 0;
}


vx_status vxBox3x3(vx_node node, vx_image src, vx_image dst, vx_border_t *bordermode)
{
	return commonOneIOneO(node, src, dst, wrap_fcvBoxFilter3x3u8);
}

static fcvStatus wrap_fcvFilterGaussian3x3u8_v2( const uint8_t* __restrict src,
                           unsigned int              srcWidth,
                           unsigned int              srcHeight,
                           unsigned int              srcStride,
                           uint8_t* __restrict       dst,
                           unsigned int              dstStride)
{
	fcvFilterGaussian3x3u8_v2(src, srcWidth, srcHeight, srcStride, dst, dstStride, 1);
	return 0;
}

vx_status vxGaussian3x3(vx_node node, vx_image src, vx_image dst, vx_border_t *bordermode)
{
	return commonOneIOneO(node, src, dst, wrap_fcvFilterGaussian3x3u8_v2);
}
