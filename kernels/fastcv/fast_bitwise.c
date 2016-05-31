/*
 * Copyright (c) 2016 National Tsing Hua University.
 * \brief The FastCV OpenVX Kernel implementation
 * \author Tzu-Hsiang Lin <thlin@pllab.cs.nthu.edu.tw>
 */

#include "fastcv_model.h"
#include "vx_fastcv.h"
#include "fast_common.h"

vx_status vxAnd(vx_node node, vx_image in1, vx_image in2, vx_image output)
{
	return commonTwoIOneO(node, in1, in2, output, fcvBitwiseAndu8);
}

vx_status vxXor(vx_node node, vx_image in1, vx_image in2, vx_image output)
{
	return commonTwoIOneO(node, in1, in2, output, fcvBitwiseXoru8);
}

vx_status vxNot(vx_node node, vx_image input, vx_image output)
{
	return commonOneIOneO(node, input, output, fcvBitwiseNotu8);
}
