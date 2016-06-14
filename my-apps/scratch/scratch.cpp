#include <VX/vx.h>

int main(int argc, char **argv)
{
	vx_context context = vxCreateContext();
	vx_uint32  width = 640;
	vx_uint32  height = 480;
	vx_image input_rgb_image = vxCreateImage(context, width, height, VX_DF_IMAGE_RGB);
	vx_image yuv_image = vxCreateImage(context, width, height, VX_DF_IMAGE_IYUV);
	vx_graph graph = vxCreateGraph(context);

	vxColorConvertNode(graph, input_rgb_image, yuv_image);
	vxVerifyGraph(graph);
	vxProcessGraph(graph);

	vxReleaseImage(&yuv_image);
	vxReleaseImage(&input_rgb_image);
	vxReleaseGraph(&graph);
	vxReleaseContext(&context);
	return 0;
}