#include "VirtualImage.hpp"

using namespace OpenVX;

VirtualImage::VirtualImage(Graph &rGraph,
	vx_uint32 width,
	vx_uint32 height,
	vx_df_image color)
{
	m_image = vxCreateVirtualImage(rGraph.getVxGraph(), width, height, color);
	GET_STATUS_CHECK(m_image);
}

VirtualImage::~VirtualImage()
{
	vxReleaseImage(&m_image);
}

vx_image VirtualImage::getVxImage() const
{
	return m_image;
}

vx_uint32 VirtualImage::width()
{
	vx_uint32 v;
	vxQueryImage(m_image, VX_IMAGE_WIDTH, &v, sizeof(v));
	return v;
}

vx_uint32 VirtualImage::height()
{
	vx_uint32 v;
	vxQueryImage(m_image, VX_IMAGE_HEIGHT, &v, sizeof(v));
	return v;
}

vx_df_image VirtualImage::format()
{
	vx_df_image v;
	vxQueryImage(m_image, VX_IMAGE_FORMAT, &v, sizeof(v));
	return v;
}

vx_enum VirtualImage::range()
{
	vx_enum v;
	vxQueryImage(m_image, VX_IMAGE_RANGE, &v, sizeof(v));
	return v;
}

vx_enum VirtualImage::space()
{
	vx_enum v;
	vxQueryImage(m_image, VX_IMAGE_SPACE, &v, sizeof(v));
	return v;
}

vx_uint32 VirtualImage::planes()
{
	vx_size v;
	vxQueryImage(m_image, VX_IMAGE_PLANES, &v, sizeof(v));
	return v;
}
