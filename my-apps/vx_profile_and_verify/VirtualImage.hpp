#ifndef _VIRTUAL_IMAGE_HPP_
#define _VIRTUAL_IMAGE_HPP_
#include"vx.hpp"

namespace OpenVX {

	class VirtualImage
	{
		vx_image m_image;

	public:
		VirtualImage(Graph &rGraph, vx_uint32 width, vx_uint32 height, vx_df_image color);  // vxCreateVirtualImage                                                                   // vxCreateImageFromROI
		~VirtualImage();
		vx_image getVxImage() const;
		
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
