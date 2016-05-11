#include "Application.hpp"

using namespace OpenVX;
using namespace cv;

AppTwoIOneO::AppTwoIOneO(Context &context, vx_kernel_e kernel_e)
	: Application(context), mKernel_e(kernel_e), resultVX(NULL)
{
}

AppTwoIOneO::~AppTwoIOneO()
{
}

void AppTwoIOneO::prepareInput()
{
	src1 = imread(SRC_IMG_NAME1);
	NULLPTR_CHECK(src1.data);
	resize(src1, src1, Size(IMG_WIDTH, IMG_HEIGHT));
	cvtColor(src1, src1, CV_RGB2GRAY);

	src2 = imread(SRC_IMG_NAME2);
	NULLPTR_CHECK(src2.data);
	resize(src2, src2, Size(IMG_WIDTH, IMG_HEIGHT));
	cvtColor(src2, src2, CV_RGB2GRAY);

	in1 = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8, src1);
	in2 = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8, src2);
	out = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8);
}

void AppTwoIOneO::process()
{
	Node *node = mGraph->addNode(mKernel_e, TARGET_OPENCL);
	node->connect(3, in1->getVxImage(), in2->getVxImage(), out->getVxImage());
	if (mGraph->verify())
		mGraph->process();

	out->getCvMat(&resultVX);

	mGraph->removeNode(node);
}

bool AppTwoIOneO::verify()
{
	Mat *resultGolden;
	Node *node = mGraph->addNode(mKernel_e, TARGET_C_MODEL);
	node->connect(3, in1->getVxImage(), in2->getVxImage(), out->getVxImage());
	if (mGraph->verify())
		mGraph->process();
	out->getCvMat(&resultGolden);
	mGraph->removeNode(node);

	return verifyTwoMat(*resultGolden, *resultVX);
}

void AppTwoIOneO::release()
{
	delete in1;
	delete in2;
	delete out;
	if (resultVX != NULL)
		delete resultVX;
}
