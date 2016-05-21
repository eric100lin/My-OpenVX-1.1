#include "Application.hpp"

using namespace OpenVX;
using namespace cv;

Application::Application(Context &context, vx_kernel_e kernel_e) 
	: mContext(context), mKernel_e(kernel_e)
{
	mGraph = new Graph(context);
}

Application::~Application()
{
	delete mGraph;
}

std::string Application::getKernelFullName(enum Target target_e)
{
	return Kernel::getFullKernelName(mKernel_e, target_e);
}

bool Application::verifyTwoMat(Mat inMat, Mat resultMat)
{
	int w, h, cnt = 0;
	int width = inMat.cols;
	int height = inMat.rows;
	unsigned char diff, max_diff=0, min_diff=255;
	long int sum_diff=0;

	unsigned char *ptr_inMat = inMat.data;
	unsigned char *ptr_resMat = resultMat.data;
	for (h = 0; h<height; h++)
	{
		for (w = 0; w<width; w++, ptr_inMat++, ptr_resMat++)
		{
			if (h == 0 || w == 0 || h == height - 1 || w == width - 1)
				continue;	//Don't verify boarder
			
			diff = *ptr_resMat - *ptr_inMat;
			if (diff != 0)
			{
				if (++cnt < 11)
				{
					printf(" fail at inMat[%d,%d]:%d != resultMat[%d,%d]:%d diff:%d\n",
						h, w, *ptr_inMat, h, w, *ptr_resMat, diff);
				}
				if(diff>max_diff)	max_diff = diff;
				if(diff<min_diff)	min_diff = diff;
				sum_diff += diff;
			}
		}
	}
	if(cnt != 0)
	{
		int total_pixel = height*width;
		float percent = cnt*100.0f / total_pixel;
		printf(" - error pixel: %d/%d (%f %%)\n", cnt, total_pixel, percent);
		printf(" - max_diff: %d\n", max_diff);
		printf(" - min_diff: %d\n", min_diff);
		printf(" - average_diff: %d\n", (int)(sum_diff/cnt));
	}
	return (cnt==0);
}
