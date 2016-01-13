#include <stdio.h>
#include <stdlib.h>
#include <VX/vx.h>
#include <vx_debug.h>
#define SRC_IMG_NAME "lena.jpg"
#define IMG_WIDTH 640
#define IMG_HEIGHT 480
#define VX_CL_NOT_NAME    "pc.opencl:org.khronos.openvx.not"
#define VX_CL_BOX3X3_NAME "pc.opencl:org.khronos.openvx.box3x3"
#define CHECK_NOT_NULL(value, name) 										  \
{ 																			  \
	if (value == NULL) 												  		  \
	{																		  \
		printf("ERROR: %s got NULL in function %s\n", name, __FUNCTION__);    \
		return VX_FAILURE;													  \
	}																		  \
}
#define CHECK_STATUS(status, vx_func) 										  \
{ 																			  \
	if (status != VX_SUCCESS) 												  \
	{																		  \
		printf("%s ERROR:%d in function %s\n", vx_func, status, __FUNCTION__);\
		return status;														  \
	}																		  \
}
#include "vx_cv_interoperate.h"
#include "not_box3x3_graph.h"
#include "not_not_graph.h"
bool verify_result(Mat inMat, Mat resultMat);

int main(int argc, char **argv)
{
	int i;
	vx_status status;
//	vx_set_debug_zone(VX_ZONE_ERROR);
//	vx_set_debug_zone(VX_ZONE_WARNING);
//	vx_set_debug_zone(VX_ZONE_INFO);

	vx_context context = vxCreateContext();
	CHECK_NOT_NULL(context, "vxCreateContext");
	printf("Success create vx_context!!\n\n");

	Mat src = imread(SRC_IMG_NAME);
	CHECK_NOT_NULL(src.data, "imread");
	resize(src, src, Size(IMG_WIDTH,IMG_HEIGHT));
	cvtColor(src, src, CV_RGB2GRAY);
	
	for(i=0; i<10; i++)
	{
		Mat result(IMG_HEIGHT,IMG_WIDTH,CV_8UC1);
		printf("Start to run not_box3x3_graph()\n");
		//status = not_box3x3_graph(context, src.clone(), result);
		//printf("Return from not_box3x3_graph() result: %d\n", status);
		status = not_not_graph(context, src.clone(), result);
		printf("Return from not_not_graph() result: %d\n", status);
		if(verify_result(src.clone(), result))
			printf("Verify passed!!\n");
		else
			printf("Verify fail!!\n");
		printf("\n");
	}
	
	status = vxReleaseContext(&context);
	CHECK_STATUS(status, "vxReleaseContext");
	printf("%s done!!\n", argv[0]);
	return 0;
}

bool verify_result(Mat inMat, Mat resultMat)
{
	int w, h;
	int width = inMat.cols;
	int height = inMat.rows;
	bitwise_not(inMat,inMat);
	bitwise_not(inMat,inMat);
	//blur(inMat, inMat, Size(3,3));
	//boxFilter(inMat, inMat, inMat.depth(), Size(3,3));
	
	unsigned char *ptr_inMat = inMat.data;
	unsigned char *ptr_resMat = resultMat.data;
	for(h=0; h<height; h++)
	{
		for(w=0; w<width; w++, ptr_inMat++, ptr_resMat++)
		{
			if(*ptr_inMat != *ptr_resMat)
			{
				printf("fail at inMat[%d,%d]:%d != resultMat[%d,%d]:%d\n",
					h,w,*ptr_inMat,h,w,*ptr_resMat);
				return false;
			}
		}
	}
	return true;
}