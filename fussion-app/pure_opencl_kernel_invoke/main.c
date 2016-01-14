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
	int cnt = 0;
	
	unsigned char *ptr_in_img = in_img->imageData;
	unsigned char *ptr_resMat = result_img->imageData;
	for(h=0; h<height; h++)
	{
		for(w=0; w<width; w++, ptr_in_img++, ptr_resMat++)
		{
			int in = *ptr_in_img;
			int res = *ptr_resMat;
			int diff = in - res;
			if(diff<=1 && diff>=-1)
				continue;
			
			printf("diff:%d fail at in_img[%d,%d]:%d != result_img[%d,%d]:%d\n",diff,
				h,w,*ptr_in_img,h,w,*ptr_resMat);
			++cnt;
			//if(++cnt>20)
			//return 0;
		}
	}
	if(cnt!=0)	return 0;
	return 1;
}

void print30x30(IplImage *in_img, IplImage *result_img)
{
	int w, h;
	int width = in_img->width;
	int height = in_img->height;
	
	unsigned char *ptr_in_img = in_img->imageData;
	unsigned char *ptr_resMat = result_img->imageData;
	for(h=0; h<height; h++)
	{
		for(w=0; w<width; w++, ptr_in_img++)
		{
			if(w<30 && h<30)
				printf("%3d ", *ptr_in_img);
		}
		if(h<30)	printf("\n");
	}
	printf("\n");
	
	for(h=0; h<height; h++)
	{
		for(w=0; w<width; w++, ptr_resMat++)
		{
			if(w<30 && h<30)
				printf("%3d ", *ptr_resMat);
		}
		if(h<30)	printf("\n");
	}
}

unsigned int round_div(unsigned int dividend, unsigned int divisor)
{
    return (dividend + (divisor / 2)) / divisor;
}

void my_box3x3(IplImage *in_img, IplImage *result_img)
{
	int x, y;
	unsigned int width = in_img->width;
	unsigned int height = in_img->height;
	unsigned int sx = 1;
	unsigned int sy = width;
	unsigned int TY=1, TX=16;
	#define vxImagePixel(ptr, x, y, sx, sy) \
		(((unsigned char *)ptr)[((y) * sy) + ((x) * sx)])
	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			unsigned int sum = 0;
			if (y == 0 || x == 0 || x == (width - 1) || y == (height - 1))
			{
				vxImagePixel(result_img->imageData, x, y, sx, sy) = 
					vxImagePixel(in_img->imageData, x, y, sx, sy);
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
			
			if(y==TY && x==TX) printf("\nsum=%d sum/9=%d\n", sum, round_div(sum, 9));
			sum = round_div(sum, 9);
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
		my_box3x3(cloned_img, cloned_img);
		//cvNot(cloned_img,cloned_img);
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