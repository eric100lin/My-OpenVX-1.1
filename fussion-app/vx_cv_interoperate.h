#ifndef _VX_CV_INTEROPERATE_H_
#define _VX_CV_INTEROPERATE_H_
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;

vx_status WriteImage(Mat source, vx_image vImage)
{
	int plane = 0;
	int width = source.cols;
	int height = source.rows;
	vx_rectangle_t rect;
	rect.start_x = rect.start_y = 0;
	rect.end_x = width;
	rect.end_y = height;
	vx_imagepatch_addressing_t addr = { 0 };
	vx_status status = VX_SUCCESS;
	vx_uint8 *src = NULL;

	status = vxAccessImagePatch(vImage, &rect, plane, &addr, (void **)&src, VX_WRITE_ONLY);
	if (status == VX_SUCCESS)
	{
		memcpy(src, source.data, width*height*sizeof(vx_uint8));
		status = vxCommitImagePatch(vImage, &rect, plane, &addr, src);
	}
	return status;
}

vx_status ReadImage(Mat &dst, vx_image vImage)
{
	int plane = 0;
	int width = dst.cols;
	int height = dst.rows;
	vx_rectangle_t rect;
	rect.start_x = rect.start_y = 0;
	rect.end_x = width;
	rect.end_y = height;
	vx_imagepatch_addressing_t addr = { 0 };
	vx_status status = VX_SUCCESS;
	vx_uint8 *src = NULL;

	status = vxAccessImagePatch(vImage, &rect, plane, &addr, (void **)&src, VX_READ_ONLY);
	if (status == VX_SUCCESS)
	{
		memcpy(dst.data, src, width*height*sizeof(vx_uint8));
		status = vxCommitImagePatch(vImage, &rect, plane, &addr, src);
	}
	return status;
}

#endif