#include "OpenCV_data_translator.hpp"

using namespace OpenVX;
using namespace cv;

/************************************************************************************************************
Converting CV Pyramid into an OpenVX Pyramid
*************************************************************************************************************/
int OpenVX::CV_to_VX_Pyramid(vx_pyramid pyramid_vx, vector<Mat> pyramid_cv)
{
	vx_status status = VX_SUCCESS;
	vx_size Level_vx = 0; vx_uint32 width = 0; 	vx_uint32 height = 0; vx_int32 i;

	ERROR_CHECK(vxQueryPyramid(pyramid_vx, VX_PYRAMID_ATTRIBUTE_LEVELS, &Level_vx, sizeof(Level_vx)));
	for (i = 0; i < (int)Level_vx; i++)
	{
		vx_image this_level = vxGetPyramidLevel(pyramid_vx, i);
		ERROR_CHECK(vxQueryImage(this_level, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width)));
		ERROR_CHECK(vxQueryImage(this_level, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height)));
		if (width != pyramid_cv[i].cols && height != pyramid_cv[i].rows)
		{
			vxAddLogEntry((vx_reference)pyramid_vx, VX_ERROR_INVALID_DIMENSION, "CV_to_VX_Pyramid ERROR: Pyramid Image Mismatch\n"); return VX_ERROR_INVALID_DIMENSION;
		}
		Mat* pyr_level;
		pyr_level = &pyramid_cv[i];
		CV_to_VX_Image(this_level, pyr_level);
	}
	return 0;
}

/************************************************************************************************************
Converting VX matrix into an OpenCV Mat
*************************************************************************************************************/
int OpenVX::VX_to_CV_MATRIX(Mat** mat, vx_matrix matrix_vx)
{
	vx_status status = VX_SUCCESS;
	vx_size numRows = 0; vx_size numCols = 0; vx_enum type; int Type_CV = 0;

	ERROR_CHECK(vxQueryMatrix(matrix_vx, VX_MATRIX_ATTRIBUTE_ROWS, &numRows, sizeof(numRows)));
	ERROR_CHECK(vxQueryMatrix(matrix_vx, VX_MATRIX_ATTRIBUTE_COLUMNS, &numCols, sizeof(numCols)));
	ERROR_CHECK(vxQueryMatrix(matrix_vx, VX_MATRIX_ATTRIBUTE_TYPE, &type, sizeof(type)));

	if (type == VX_TYPE_INT32)Type_CV = CV_32S;
	if (type == VX_TYPE_FLOAT32)Type_CV = CV_32F;

	if (type != VX_TYPE_FLOAT32 && type != VX_TYPE_INT32)
	{
		vxAddLogEntry((vx_reference)matrix_vx, VX_ERROR_INVALID_FORMAT, "VX_to_CV_MATRIX ERROR: Matrix type not Supported in this RELEASE\n"); return VX_ERROR_INVALID_FORMAT;
	}

	Mat * m_cv;	m_cv = new Mat((int)numRows, (int)numCols, Type_CV); vx_size mat_size = numRows * numCols;
	float *dyn_matrix = new float[mat_size]; int z = 0;

	ERROR_CHECK(vxReadMatrix(matrix_vx, (void *)dyn_matrix));
	for (int i = 0; i < (int)numRows; i++)
		for (int j = 0; j < (int)numCols; j++)
		{
			m_cv->at<float>(i, j) = dyn_matrix[z]; z++;
		}

	*mat = m_cv;
	return status;
}

/************************************************************************************************************
Converting VX Image into an OpenCV Mat
*************************************************************************************************************/
int OpenVX::VX_to_CV_Image(Mat** mat, vx_image image)
{
	vx_status status = VX_SUCCESS;
	vx_uint32 width = 0; vx_uint32 height = 0; vx_df_image format = VX_DF_IMAGE_VIRT; int CV_format = 0; vx_size planes = 0;

	ERROR_CHECK(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width)));
	ERROR_CHECK(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height)));
	ERROR_CHECK(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format)));
	ERROR_CHECK(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_PLANES, &planes, sizeof(planes)));

	if (format == VX_DF_IMAGE_U8)CV_format = CV_8U;
	if (format == VX_DF_IMAGE_S16)CV_format = CV_16S;
	if (format == VX_DF_IMAGE_RGB)CV_format = CV_8UC3;

	if (format != VX_DF_IMAGE_U8 && format != VX_DF_IMAGE_S16 && format != VX_DF_IMAGE_RGB)
	{
		vxAddLogEntry((vx_reference)image, VX_ERROR_INVALID_FORMAT, "VX_to_CV_Image ERROR: Image type not Supported in this RELEASE\n"); return VX_ERROR_INVALID_FORMAT;
	}

	Mat * m_cv;	m_cv = new Mat(height, width, CV_format); Mat *pMat = (Mat *)m_cv;
	vx_rectangle_t rect; rect.start_x = 0; rect.start_y = 0; rect.end_x = width; rect.end_y = height;

	vx_uint8 *src[4] = { NULL, NULL, NULL, NULL }; vx_uint32 p; void *ptr = NULL;
	vx_imagepatch_addressing_t addr[4] = { 0, 0, 0, 0 }; vx_uint32 y = 0u;

	for (p = 0u; (p < (int)planes); p++)
	{
		ERROR_CHECK(vxAccessImagePatch(image, &rect, p, &addr[p], (void **)&src[p], VX_READ_ONLY));
		size_t len = addr[p].stride_x * (addr[p].dim_x * addr[p].scale_x) / VX_SCALE_UNITY;
		for (y = 0; y < height; y += addr[p].step_y)
		{
			ptr = vxFormatImagePatchAddress2d(src[p], 0, y - rect.start_y, &addr[p]);
			memcpy(pMat->data + y * pMat->step, ptr, len);
		}
	}

	for (p = 0u; p < (int)planes; p++)
		ERROR_CHECK(vxCommitImagePatch(image, &rect, p, &addr[p], src[p]));

	*mat = pMat;

	return status;
}

/************************************************************************************************************
Converting CV Image into an OpenVX Image
*************************************************************************************************************/
int OpenVX::CV_to_VX_Image(vx_image image, Mat* mat)
{
	vx_status status = VX_SUCCESS; vx_uint32 width = 0; vx_uint32 height = 0; vx_size planes = 0;

	ERROR_CHECK(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width)));
	ERROR_CHECK(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height)));
	ERROR_CHECK(vxQueryImage(image, VX_IMAGE_ATTRIBUTE_PLANES, &planes, sizeof(planes)));

	Mat *pMat = mat; vx_rectangle_t rect; rect.start_x = 0; rect.start_y = 0; rect.end_x = width; rect.end_y = height;

	vx_uint8 *src[4] = { NULL, NULL, NULL, NULL }; vx_uint32 p; void *ptr = NULL;
	vx_imagepatch_addressing_t addr[4] = { 0, 0, 0, 0 }; vx_uint32 y = 0u;

	for (p = 0u; (p <(int)planes); p++)
	{
		ERROR_CHECK(vxAccessImagePatch(image, &rect, p, &addr[p], (void **)&src[p], VX_READ_ONLY));
		size_t len = addr[p].stride_x * (addr[p].dim_x * addr[p].scale_x) / VX_SCALE_UNITY;
		for (y = 0; y < height; y += addr[p].step_y)
		{
			ptr = vxFormatImagePatchAddress2d(src[p], 0, y - rect.start_y, &addr[p]);
			memcpy(ptr, pMat->data + y * pMat->step, len);
		}
	}

	for (p = 0u; p < (int)planes; p++)
		ERROR_CHECK(vxCommitImagePatch(image, &rect, p, &addr[p], src[p]));

	return status;
}
