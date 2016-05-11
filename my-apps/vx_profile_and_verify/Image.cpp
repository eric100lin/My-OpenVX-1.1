#include "Image.hpp"

using namespace OpenVX;
using namespace cv;

Image::Image(Context &rContext,
			 vx_uint32 width,
			 vx_uint32 height,
			 vx_df_image color)
{
	m_image = vxCreateImage(rContext.getVxContext(), width, height, color);
	GET_STATUS_CHECK(m_image);
}

Image::Image(Context &rContext, vx_uint32 width, vx_uint32 height, vx_df_image color, cv::Mat &mat)
	: Image(rContext, width, height, color)
{
	CV_to_VX_Image(m_image, &mat);
}

Image::Image(Context &rContext,
			 vx_df_image color,
			 vx_imagepatch_addressing_t addrs[],
			 void* ptrs[],
			 vx_enum type)
{
	m_image = vxCreateImageFromHandle(rContext.getVxContext(), color, addrs, ptrs, type);
	GET_STATUS_CHECK(m_image);
}

Image::Image(Context &rContext,
	vx_uint32 width,
	vx_uint32 height,
	vx_df_image color,
	vx_pixel_value_t* value)
{
	m_image = vxCreateUniformImage(rContext.getVxContext(), width, height, color, value);
	GET_STATUS_CHECK(m_image);
}

Image::Image(Image* pImg,
			 vx_rectangle_t *rect)
{
	m_image = vxCreateImageFromROI(pImg->m_image, rect);
	GET_STATUS_CHECK(m_image);
}

Image::~Image()
{
	vxReleaseImage(&m_image);
}

void Image::getCvMat(Mat **mat)
{
	VX_to_CV_Image(mat, m_image);
}

vx_image Image::getVxImage() const
{
	return m_image;
}

vx_status Image::AccessImagePatch(vx_rectangle_t *rect, vx_uint32 p, vx_imagepatch_addressing_t *addr, void **ptr, vx_enum usage)
{
	return vxAccessImagePatch(m_image, rect, p, addr, ptr, usage);
}

vx_status Image::CommitImagePatch(vx_rectangle_t *rect, vx_uint32 p, vx_imagepatch_addressing_t *addr, void *ptr)
{
	return vxCommitImagePatch(m_image, rect, p, addr, ptr);
}

vx_size Image::ComputeImagePatchSize(vx_rectangle_t *rect, vx_uint32 p)
{
	return vxComputeImagePatchSize(m_image, rect, p);
}

void* Image::FormatImagePatchAddress1d(void *ptr, vx_uint32 index, vx_imagepatch_addressing_t* addr)
{
	return vxFormatImagePatchAddress1d(ptr, index, addr);
}

void* Image::FormatImagePatchAddress2d(void *ptr, vx_uint32 x, vx_uint32 y, vx_imagepatch_addressing_t* addr)
{
	return vxFormatImagePatchAddress2d(ptr, x, y, addr);
}

vx_rectangle_t Image::GetValidRegionImage(Image* pImage)
{
	vx_rectangle_t rect = {0, 0, 0, 0};
	vxGetValidRegionImage(m_image, &rect);
	return rect;
}

vx_uint32 Image::width()
{
	vx_uint32 v;
	vxQueryImage(m_image, VX_IMAGE_WIDTH, &v, sizeof(v));
	return v;
}

vx_uint32 Image::height()
{
	vx_uint32 v;
	vxQueryImage(m_image, VX_IMAGE_HEIGHT, &v, sizeof(v));
	return v;
}

vx_df_image Image::format()
{
	vx_df_image v;
	vxQueryImage(m_image, VX_IMAGE_FORMAT, &v, sizeof(v));
	return v;
}

vx_enum Image::range()
{
	vx_enum v;
	vxQueryImage(m_image, VX_IMAGE_RANGE, &v, sizeof(v));
	return v;
}

vx_enum Image::space()
{
	vx_enum v;
	vxQueryImage(m_image, VX_IMAGE_SPACE, &v, sizeof(v));
	return v;
}

vx_uint32 Image::planes()
{
	vx_size v;
	vxQueryImage(m_image, VX_IMAGE_PLANES, &v, sizeof(v));
	return v;
}
