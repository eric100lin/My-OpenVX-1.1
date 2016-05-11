#ifndef _OPENCV_DATA_TRANSLATOR_HPP_
#define _OPENCV_DATA_TRANSLATOR_HPP_
#include "vx.hpp"

namespace OpenVX
{

	int CV_to_VX_Pyramid(vx_pyramid pyramid_vx, std::vector<cv::Mat> pyramid_cv);

	int VX_to_CV_MATRIX(cv::Mat** mat, vx_matrix matrix_vx);

	int VX_to_CV_Image(cv::Mat** mat, vx_image image);

	int CV_to_VX_Image(vx_image image, cv::Mat* mat);
}

#endif