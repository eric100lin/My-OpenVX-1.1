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

int verify_result_U8(IplImage *in_img, IplImage *result_img)
{
	int w, h;
	int width = in_img->width;
	int height = in_img->height;
	int cnt = 0;
	
	unsigned char *ptr_in_img = in_img->imageData;
	unsigned char *ptr_resMat = result_img->imageData;
	for(h=0; h<height; h++)
	{
		for(w=0; w<width; w++, ptr_in_img++, ptr_resMat++)
		{
			if(*ptr_in_img != *ptr_resMat)
			{
			//int in = *ptr_in_img;
			//int res = *ptr_resMat;
			//int diff = in - res;
			//if(diff<=1 && diff>=-1)
			//	continue;
			
			//++cnt;
				printf("fail at in_img[%d,%d]:%d != result_img[%d,%d]:%d\n",
					h,w,*ptr_in_img,h,w,*ptr_resMat);
				
				if(++cnt>3)
					return 0;
			}
		}
	}
	//if(cnt!=0)	return 0;
	return 1;
}

int verify_result_U16(IplImage *in_img, IplImage *result_img)
{
	int w, h;
	int width = in_img->width;
	int height = in_img->height;
	int cnt = 0;
	
	int16_t *ptr_in_img = (int16_t *)in_img->imageData;
	int16_t *ptr_resMat = (int16_t *)result_img->imageData;
	for(h=0; h<height; h++)
	{
		for(w=0; w<width; w++, ptr_in_img++, ptr_resMat++)
		{
			//int in = *ptr_in_img;
			//int res = *ptr_resMat;
			//int diff = in - res;
			//if(diff<=1 && diff>=-1)
			//	continue;
			
			//++cnt;
			//if(++cnt>20)
			if(*ptr_in_img != *ptr_resMat)
			{
				printf("fail at in_img[%d,%d]:%d != result_img[%d,%d]:%d\n",
					h,w,*ptr_in_img,h,w,*ptr_resMat);
					
				if(++cnt>3)
					return 0;
			}
		}
	}
	//if(cnt!=0)	return 0;
	return 1;
}

void print10x10_U8(IplImage *img)
{
	int w, h;
	int width = img->width;
	int height = img->height;
	
	unsigned char *ptr_img = (unsigned char *)img->imageData;
	for(h=0; h<height; h++)
	{
		for(w=0; w<width; w++, ptr_img++)
		{
			if(w<10 && h<10)
				printf("%5u ", *ptr_img);
		}
		if(h<10)	printf("\n");
	}
	printf("\n");
}

void print10x10_U16(IplImage *img)
{
	int w, h;
	int width = img->width;
	int height = img->height;
	
	int16_t *ptr_img = (int16_t *)img->imageData;
	for(h=0; h<height; h++)
	{
		for(w=0; w<width; w++, ptr_img++)
		{
			if(w<10 && h<10)
				printf("%5u ", *ptr_img);
		}
		if(h<10)	printf("\n");
	}
	printf("\n");
}

unsigned int round_div(unsigned int dividend, unsigned int divisor)
{
    return (dividend + (divisor / 2)) / divisor;
}

void my_box3x3(IplImage *in_img, IplImage *result_img, IplImage *sum_cpu)
{
	int x, y;
	int width = in_img->width;
	int height = in_img->height;
	int sx = 1;
	int sy = width;
	int TY=1, TX=8;
	int16_t *ptr_sum = (int16_t *)sum_cpu->imageData;
	printf("TY=%d, TX=%d\n", TY, TX);
	
	#define vxImagePixel(ptr, x, y, sx, sy) \
		(((unsigned char *)ptr)[((y) * sy) + ((x) * sx)])
	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			int16_t sum = 0;
			if (y == 0 || x == 0 || x == (width - 1) || y == (height - 1))
			{
				vxImagePixel(result_img->imageData, x, y, sx, sy) = 
					vxImagePixel(in_img->imageData, x, y, sx, sy);
				int16_t cast_input = (int16_t)vxImagePixel(in_img->imageData, x, y, sx, sy);
				ptr_sum[((y) * sy) + ((x) * sx)] = cast_input;
				continue; // border mode...
			}
			sum += vxImagePixel(in_img->imageData, x-1, y-1, sx, sy);
			if(y==TY && x==TX) printf("%d ", vxImagePixel(in_img->imageData, x-1, y-1, sx, sy));
			sum += vxImagePixel(in_img->imageData, x+0, y-1, sx, sy);
			if(y==TY && x==TX) printf("%d ", vxImagePixel(in_img->imageData, x+0, y-1, sx, sy));
			sum += vxImagePixel(in_img->imageData, x+1, y-1, sx, sy);
			if(y==TY && x==TX) printf("%d ", vxImagePixel(in_img->imageData, x+1, y-1, sx, sy));
			sum += vxImagePixel(in_img->imageData, x-1, y+0, sx, sy);
			if(y==TY && x==TX) printf("%d ", vxImagePixel(in_img->imageData, x-1, y+0, sx, sy));
			sum += vxImagePixel(in_img->imageData, x+0, y+0, sx, sy);
			if(y==TY && x==TX) printf("%d ", vxImagePixel(in_img->imageData, x+0, y+0, sx, sy));
			sum += vxImagePixel(in_img->imageData, x+1, y+0, sx, sy);
			if(y==TY && x==TX) printf("%d ", vxImagePixel(in_img->imageData, x+1, y+0, sx, sy));
			sum += vxImagePixel(in_img->imageData, x-1, y+1, sx, sy);
			if(y==TY && x==TX) printf("%d ", vxImagePixel(in_img->imageData, x-1, y+1, sx, sy));
			sum += vxImagePixel(in_img->imageData, x+0, y+1, sx, sy);
			if(y==TY && x==TX) printf("%d ", vxImagePixel(in_img->imageData, x+0, y+1, sx, sy));
			sum += vxImagePixel(in_img->imageData, x+1, y+1, sx, sy);
			if(y==TY && x==TX) printf("%d ", vxImagePixel(in_img->imageData, x+1, y+1, sx, sy));
			
			if(y==TY && x==TX) printf("\nsum=%d sum/9.0=%.3f sum/9=%d round_div(sum, 9)=%d\n", sum, sum/9.0f, sum/9, round_div(sum, 9));
			ptr_sum[((y) * sy) + ((x) * sx)] = sum;
			//sum = round_div(sum, 9);
			sum /= 9;
			vxImagePixel(result_img->imageData, x, y, sx, sy) = (unsigned char)sum;
		}
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
	
	printf("Original image\n");
	print10x10_U8(src_gray);
			
	printf("Start to run boxFilter_not_graph()\n");
	for(i=0; i<10; i++)
	{
		IplImage *cloned_img;
		IplImage *result_cpu = cvCreateImage(dst_cvsize, IPL_DEPTH_8U, 1);
		IplImage *result_gpu = cvCreateImage(dst_cvsize, IPL_DEPTH_8U, 1);
		IplImage *sum_gpu = cvCreateImage(dst_cvsize, IPL_DEPTH_16U, 1);
		IplImage *sum_cpu = cvCreateImage(dst_cvsize, IPL_DEPTH_16U, 1);
		printf("Iteration:%d\n", i);
		
		cloned_img = cvCloneImage(src_gray);
		int status = boxFilter_not_graph(cloned_img, result_gpu, sum_gpu);
		cvReleaseImage(&cloned_img);
		
		printf("Return from boxFilter_not_graph() result: %d\n", status);
		
		cloned_img = cvCloneImage(src_gray);
		
		my_box3x3(cloned_img, result_cpu, sum_cpu);
		//cvNot(cloned_img,cloned_img);
		
		printf("verify_result_U16(sum_cpu, sum_gpu)\n");
		verify_result_U16(sum_cpu, sum_gpu);
		
		printf("verify_result_U8(result_cpu, result_gpu)\n");
		if(verify_result_U8(result_cpu, result_gpu))
			printf("Verify passed!!\n");
		else
		{
			printf("Verify fail!!\n");
		}
		
		if(i==9)
		{
			printf("Original image\n");
			print10x10_U8(src_gray);
			printf("box3x3 result from CPU\n");
			print10x10_U8(result_cpu);
			printf("box3x3 sum from CPU\n");
			print10x10_U16(sum_cpu);
			printf("box3x3 result from GPU\n");
			print10x10_U8(result_gpu);
			printf("box3x3 sum from GPU\n");
			print10x10_U16(sum_gpu);
		}
		
		printf("\n");
		cvReleaseImage(&cloned_img);
		cvReleaseImage(&result_gpu);
		cvReleaseImage(&result_cpu);
		cvReleaseImage(&sum_gpu);
		cvReleaseImage(&sum_cpu);
	}
	
	cvReleaseImage(&src);
	cvReleaseImage(&src_resized);
	cvReleaseImage(&src_gray);
	printf("%s done!!\n", argv[0]);
	return 0;
}