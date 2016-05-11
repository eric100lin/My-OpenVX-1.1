#include "Application.hpp"

using namespace OpenVX;
using namespace cv;

Application::Application(Context &context) : mContext(context)
{
	mGraph = new Graph(context);
}

Application::~Application()
{
	delete mGraph;
}

bool Application::verifyTwoMat(Mat inMat, Mat resultMat)
{
	int w, h, cnt = 0;
	int width = inMat.cols;
	int height = inMat.rows;

	unsigned char *ptr_inMat = inMat.data;
	unsigned char *ptr_resMat = resultMat.data;
	for (h = 0; h<height; h++)
	{
		for (w = 0; w<width; w++, ptr_inMat++, ptr_resMat++)
		{
			if (h == 0 || w == 0 || h == height - 1 || w == width - 1)
				continue;	//Don't verify boarder
			if (*ptr_inMat != *ptr_resMat)
			{
				printf("fail at inMat[%d,%d]:%d != resultMat[%d,%d]:%d\n",
					h, w, *ptr_inMat, h, w, *ptr_resMat);
				if (++cnt == 10)
					return false;
			}
		}
	}
	return true;
}
