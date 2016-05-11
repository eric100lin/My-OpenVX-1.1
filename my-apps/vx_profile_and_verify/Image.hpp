#ifndef _IMAGE_HPP_
#define _IMAGE_HPP_
#include"vx.hpp"

namespace OpenVX 
{

	class Image 
	{
		vx_image m_image;

	public:
		Image(Context &rContext, vx_uint32 width, vx_uint32 height, vx_df_image color, cv::Mat &mat);
		Image(Context &rContext, vx_uint32 width, vx_uint32 height, vx_df_image color);                               // vxCreateImage
		Image(Context &rContext, vx_df_image color, vx_imagepatch_addressing_t addrs[], void* ptrs[], vx_enum type);  // vxCreateImageFromHandle
		Image(Context &rContext, vx_uint32 width, vx_uint32 height, vx_df_image color, vx_pixel_value_t* value);      // vxCreateUniformImage
		Image(Image* pImg, vx_rectangle_t *rect);                                                                     // vxCreateImageFromROI
		~Image();
		void getCvMat(cv::Mat **mat);
		vx_image getVxImage() const;

		vx_status    AccessImagePatch          (vx_rectangle_t *rect, vx_uint32 p, vx_imagepatch_addressing_t *addr, void **ptr, vx_enum usage);    // vxAccessImagePatch
		vx_status    CommitImagePatch          (vx_rectangle_t *rect, vx_uint32 p, vx_imagepatch_addressing_t *addr, void *ptr);     // vxCommitImagePatch
		vx_size      ComputeImagePatchSize     (vx_rectangle_t *rect, vx_uint32 p);                                                  // vxComputeImagePatchSize

		void*        FormatImagePatchAddress1d (void *ptr, vx_uint32 index, vx_imagepatch_addressing_t* addr);                  // vxFormatImagePatchAddress1d
		void*        FormatImagePatchAddress2d (void *ptr, vx_uint32 x, vx_uint32 y, vx_imagepatch_addressing_t* addr);         // vxFormatImagePatchAddress2d
		vx_rectangle_t GetValidRegionImage       (Image* pImage);                                                                 // vxGetValidRegionImage

		// various incarnations of vxQueryImage
		vx_uint32   width();
		vx_uint32   height();
		vx_df_image format();
		vx_uint32   planes();
		vx_enum     space();
		vx_enum     range();
	};

}
#endif

