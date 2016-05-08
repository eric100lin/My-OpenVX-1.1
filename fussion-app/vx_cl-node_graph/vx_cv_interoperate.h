#ifndef _VX_CV_INTEROPERATE_H_
#define _VX_CV_INTEROPERATE_H_
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;

vx_status WriteImage(Mat source, vx_image vImage)
{
	int image_plane_index = 0;
	int width = source.cols;
	int height = source.rows;
	vx_rectangle_t rect;
	rect.start_x = rect.start_y = 0;
	rect.end_x = width;
	rect.end_y = height;
	
	vx_imagepatch_addressing_t user_addr;
	user_addr.dim_x=(vx_uint32)width;
	user_addr.dim_y=(vx_uint32)height;
	user_addr.stride_x=1;
	user_addr.stride_y=(vx_uint32)width;
	user_addr.step_x=1;
	user_addr.step_y=1;
	user_addr.scale_x=VX_SCALE_UNITY;
	user_addr.scale_y=VX_SCALE_UNITY;

	vx_status status = VX_SUCCESS;
	status = vxCopyImagePatch(vImage, &rect, image_plane_index,
		&user_addr, (void *)source.data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
	
	return status;
}

vx_status ReadImage(Mat &dst, vx_image vImage)
{
	int image_plane_index = 0;
	int width = dst.cols;
	int height = dst.rows;
	vx_rectangle_t rect;
	rect.start_x = rect.start_y = 0;
	rect.end_x = width;
	rect.end_y = height;
	
	vx_imagepatch_addressing_t user_addr;
	user_addr.dim_x=(vx_uint32)width;
	user_addr.dim_y=(vx_uint32)height;
	user_addr.stride_x=1;
	user_addr.stride_y=(vx_uint32)width;
	user_addr.step_x=1;
	user_addr.step_y=1;
	user_addr.scale_x=VX_SCALE_UNITY;
	user_addr.scale_y=VX_SCALE_UNITY;

	vx_status status = VX_SUCCESS;
	status = vxCopyImagePatch(vImage, &rect, image_plane_index,
		&user_addr, (void *)dst.data, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
	
	return status;
}

#endif