#include <stdio.h>
#include <stdlib.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "not_not_graph.h"
#define SRC_IMG_NAME "lena.jpg"
#define IMG_WIDTH 640
#define IMG_HEIGHT 480
#define N 3*3

int verify_result(IplImage *in_img, IplImage *result_img)
{
	int w, h;
	int width = in_img->width;
	int height = in_img->height;
	
	unsigned char *ptr_in_img = in_img->imageData;
	unsigned char *ptr_resMat = result_img->imageData;
	for(h=0; h<height; h++)
	{
		for(w=0; w<width; w++, ptr_in_img++, ptr_resMat++)
		{
			if(*ptr_in_img != *ptr_resMat)
			{
				printf("fail at in_img[%d,%d]:%d != result_img[%d,%d]:%d\n",
					h,w,*ptr_in_img,h,w,*ptr_resMat);
				return 0;
			}
		}
	}
	return 1;
}

void print30x30(IplImage *in_img, IplImage *result_img)
{
	int w, h;
	int width = 30;
	int height = 30;
	
	unsigned char *ptr_in_img = in_img->imageData;
	unsigned char *ptr_resMat = result_img->imageData;
	for(h=0; h<height; h++)
	{
		for(w=0; w<width; w++, ptr_in_img++)
		{
			printf("%3d ", *ptr_in_img);
		}
		printf("\n");
	}
	printf("\n");
	
	for(h=0; h<height; h++)
	{
		for(w=0; w<width; w++, ptr_resMat++)
		{
			printf("%3d ", *ptr_resMat);
		}
		printf("\n");
	}
}

int main(int argc, char **argv)
{
	int i;

	IplImage *src = cvLoadImage(SRC_IMG_NAME, CV_LOAD_IMAGE_COLOR);
	
	CvSize dst_cvsize;
	dst_cvsize.width = IMG_WIDTH;
	dst_cvsize.height = IMG_HEIGHT;
	IplImage *src_resized = cvCreateImage(dst_cvsize, src->depth, src->nChannels);
	IplImage *src_gray = cvCreateImage(dst_cvsize, IPL_DEPTH_8U, 1);
	cvResize(src, src_resized, CV_INTER_NN);
	cvCvtColor(src_resized, src_gray, CV_RGB2GRAY);
	
	printf("Start to run not_not_graph()\n");
	for(i=0; i<10; i++)
	{
		IplImage *cloned_img;
		IplImage *result = cvCreateImage(dst_cvsize, IPL_DEPTH_8U, 1);
		printf("Iteration:%d\n", i);
		
		cloned_img = cvCloneImage(src_gray);
		int status = not_not_graph(cloned_img, result);
		cvReleaseImage(&cloned_img);
		
		printf("Return from not_not_graph() result: %d\n", status);
		
		cloned_img = cvCloneImage(src_gray);
		cvNot(cloned_img,cloned_img);
		cvNot(cloned_img,cloned_img);
		if(verify_result(cloned_img, result))
			printf("Verify passed!!\n");
		else
			printf("Verify fail!!\n");
		printf("\n");
		cvReleaseImage(&cloned_img);
		
		cvReleaseImage(&result);
	}
	
	float filter_data[N]={1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0};
	printf("Start to run boxFilter_not_graph()\n");
	for(i=0; i<1; i++)
	{
		IplImage *cloned_img;
		IplImage *result = cvCreateImage(dst_cvsize, IPL_DEPTH_8U, 1);
		printf("Iteration:%d\n", i);
		
		cloned_img = cvCloneImage(src_gray);
		int status = boxFilter_not_graph(cloned_img, result);
		cvReleaseImage(&cloned_img);
		
		printf("Return from boxFilter_not_graph() result: %d\n", status);
		
		cloned_img = cvCloneImage(src_gray);
		CvMat filter_kernel;
		cvInitMatHeader(&filter_kernel, 3, 3, CV_32F, filter_data, 3*sizeof(float));
		cvFilter2D(cloned_img, cloned_img, &filter_kernel);
		cvNot(cloned_img,cloned_img);
		if(verify_result(cloned_img, result))
			printf("Verify passed!!\n");
		else
		{
			printf("Verify fail!!\n");
			print30x30(cloned_img, result);
		}
		
		printf("\n");
		cvReleaseImage(&cloned_img);
		
		cvReleaseImage(&result);
	}
	
	cvReleaseImage(&src);
	cvReleaseImage(&src_resized);
	cvReleaseImage(&src_gray);
	printf("%s done!!\n", argv[0]);
	return 0;
}