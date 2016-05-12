#include "Application.hpp"

using namespace OpenVX;
using namespace cv;

AppOneIOneO::AppOneIOneO(Context &context, vx_kernel_e kernel_e)
	: Application(context), mKernel_e(kernel_e), resultVX(NULL)
{
}

AppOneIOneO::~AppOneIOneO()
{
}

void AppOneIOneO::prepareInput()
{
	lena_src = imread(SRC_IMG_NAME1);
	NULLPTR_CHECK(lena_src.data);
	resize(lena_src, lena_src, Size(IMG_WIDTH, IMG_HEIGHT));
	cvtColor(lena_src, lena_src, CV_RGB2GRAY);

	in = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8, lena_src);
	out = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8);
}

void AppOneIOneO::process()
{
	Node *node = mGraph->addNode(mKernel_e, TARGET_OPENCL);
	node->connect(2, in->getVxImage(), out->getVxImage());
	if (mGraph->verify())
		mGraph->process();

	out->getCvMat(&resultVX);

	mGraph->removeNode(node);
}

bool AppOneIOneO::verify()
{
	Mat *resultGolden;
	Node *node = mGraph->addNode(mKernel_e, TARGET_C_MODEL);
	node->connect(2, in->getVxImage(), out->getVxImage());
	if (mGraph->verify())
		mGraph->process();
	out->getCvMat(&resultGolden);
	mGraph->removeNode(node);

	return verifyTwoMat(*resultGolden, *resultVX);
}

void AppOneIOneO::release()
{
	delete in;
	delete out;
	if (resultVX != NULL)
		delete resultVX;
}