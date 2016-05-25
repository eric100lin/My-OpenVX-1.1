#include "Application.hpp"

using namespace OpenVX;
using namespace cv;

AppTwoIOneO::AppTwoIOneO(Context &context, vx_kernel_e kernel_e)
	: Application(context, 1, kernel_e), resultVX(NULL)
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
}

void AppTwoIOneO::setup()
{
	in1 = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8, src1);
	in2 = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8, src2);
	out = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8);
}

void AppTwoIOneO::process(int variant_numer)
{
	enum Target targets[1];
	getVariantTarget(variant_numer, targets);
	
	Node *node = mGraph->addNode(mKernel_es[0], targets[0]);
	node->connect(3, in1->getVxImage(), in2->getVxImage(), out->getVxImage());
	if (mGraph->verify())
		mGraph->process();

	out->getCvMat(&resultVX);

	mGraph->removeNode(node);
}

void AppTwoIOneO::profiling(int n_times, int variant_numer)
{
	enum Target targets[1];
	getVariantTarget(variant_numer, targets);
	
	Node *node = mGraph->addNode(mKernel_es[0], targets[0]);
	node->connect(3, in1->getVxImage(), in2->getVxImage(), out->getVxImage());
	if (!mGraph->verify())
		return;

	Node *nodes[] = { node };
	printProfilingResult(n_times, 1, nodes);
}

bool AppTwoIOneO::verify()
{
	Mat *resultGolden;
	Node *node = mGraph->addNode(mKernel_es[0], TARGET_C_MODEL);
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
	{
		delete resultVX;
		resultVX = NULL;
	}
}

void AppTwoIOneO::releaseInput()
{
	src1.release();
	src2.release();
}