#include <stdio.h>
#include <stdlib.h>
#include <VX/vx.h>
#include <vx_debug.h>
#include <VX/vx_helper.h>
#define SRC_IMG_NAME "lena.jpg"
#define IMG_WIDTH 640
#define IMG_HEIGHT 480
//#define VX_CL_NOT_NAME    "khronos.c_model:org.khronos.openvx.not"
//#define VX_CL_BOX3X3_NAME "khronos.c_model:org.khronos.openvx.box_3x3:default"
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
#include "not_graph.h"
bool verify_result(Mat inMat, Mat resultMat);

static vx_log_t helper_log;
static void vxInitLog(vx_log_t *log)
{
    log->first = -1;
    log->last = 0;
    log->count = VX_MAX_LOG_NUM_ENTRIES;
}

static void VX_CALLBACK vxHelperLogCallback(vx_context context,
                                vx_reference ref,
                                vx_status status,
                                const vx_char string[])
{
    helper_log.entries[helper_log.last].reference = ref;
    helper_log.entries[helper_log.last].status = status;
    helper_log.entries[helper_log.last].active = vx_true_e;
    strncpy(helper_log.entries[helper_log.last].message, string, VX_MAX_LOG_MESSAGE_LEN);

    if (helper_log.first == -1)
        helper_log.first = helper_log.last;
    else if (helper_log.first == helper_log.last)
        helper_log.first = (helper_log.first + 1)%helper_log.count;
    helper_log.last = (helper_log.last + 1)%helper_log.count;
}

int main(int argc, char **argv)
{
	int i;
	vx_status status;
	vx_set_debug_zone(VX_ZONE_ERROR);
	//vx_set_debug_zone(VX_ZONE_WARNING);
	//vx_set_debug_zone(VX_ZONE_INFO);

	vx_context context = vxCreateContext();
	CHECK_NOT_NULL(context, "vxCreateContext");
	printf("Success create vx_context!!\n\n");

	vxInitLog(&helper_log);
	vxRegisterLogCallback(context, &vxHelperLogCallback, vx_false_e);
	
	Mat src = imread(SRC_IMG_NAME);
	CHECK_NOT_NULL(src.data, "imread");
	resize(src, src, Size(IMG_WIDTH,IMG_HEIGHT));
	cvtColor(src, src, CV_RGB2GRAY);
	
	for(i=0; i<1; i++)
	{
		Mat result_cv(IMG_HEIGHT,IMG_WIDTH,CV_8UC1);
		Mat result_vx(IMG_HEIGHT,IMG_WIDTH,CV_8UC1);
		printf("Start to run not_box3x3_graph()\n");
		not_box3x3_cv(src.clone(), result_cv);
		status = not_box3x3_graph(context, src.clone(), result_vx);
		printf("Return from not_box3x3_graph() result_vx: %d\n", status);
		if(verify_result(result_cv, result_vx))
			printf("Verify passed!!\n");
		else
			printf("Verify fail!!\n");
		printf("\n");
		
		printf("Start to run not_not_graph()\n");
		not_not_cv(src.clone(), result_cv);
		status = not_not_graph(context, src.clone(), result_vx);
		printf("Return from not_not_graph() result_vx: %d\n", status);
		if(verify_result(result_cv, result_vx))
			printf("Verify passed!!\n");
		else
			printf("Verify fail!!\n");
		printf("\n");
		
		printf("Start to run not_graph()\n");
		not_cv(src.clone(), result_cv);
		status = not_graph(context, src.clone(), result_vx);
		printf("Return from not_not_graph() result_vx: %d\n", status);
		if(verify_result(result_cv, result_vx))
			printf("Verify passed!!\n");
		else
			printf("Verify fail!!\n");
		printf("\n");
		
		//imwrite("result_cv.jpg",result_cv);
		//imwrite("result_vx.jpg",result_vx);
	}
	
	status = vxReleaseContext(&context);
	CHECK_STATUS(status, "vxReleaseContext");
	printf("%s done!!\n", argv[0]);
	return 0;
}

bool verify_result(Mat inMat, Mat resultMat)
{
	int w, h,cnt=0;
	int width = inMat.cols;
	int height = inMat.rows;
	
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
				if(++cnt==10)
				return false;
			}
		}
	}
	return true;
}