#include "Application.hpp"
#define LUT_SIZE 256
using namespace OpenVX;
using namespace cv;

AppTableLookup::AppTableLookup(Context &context)
	: Application(context, APP_ONE_NODE, VX_KERNEL_TABLE_LOOKUP), resultVX(NULL)
{
}

AppTableLookup::~AppTableLookup()
{
}

void AppTableLookup::prepareInput()
{
	src = imread(SRC_IMG_NAME1);
	NULLPTR_CHECK(src.data);
	resize(src, src, Size(IMG_WIDTH, IMG_HEIGHT));
	cvtColor(src, src, CV_RGB2GRAY);

	lut_data = new uchar[LUT_SIZE];
	uchar *pSrc = src.data;
	memset(lut_data, 0, LUT_SIZE*sizeof(uchar));
	for (int h = 0; h < src.rows; h++)
	{
		for (int w = 0; w < src.cols; w++)
		{
			lut_data[*pSrc]++;
			pSrc++;
		}
	}
}

void AppTableLookup::setup()
{
	in = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8, src);
	out = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8);
	
	lut = vxCreateLUT(mContext.getVxContext(), VX_TYPE_UINT8, LUT_SIZE);
	GET_STATUS_CHECK(lut);
	vx_status status = vxCopyLUT(lut, lut_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
	ERROR_CHECK(status);
}

void AppTableLookup::process(int variant_numer)
{
	enum Target targets[APP_ONE_NODE];
	getVariantTarget(variant_numer, targets);
	Node *node = mGraph->addNode(mKernel_es[0], targets[0]);
	node->connect(3, in->getVxImage(), lut, out->getVxImage());
	if (mGraph->verify())
		mGraph->process();

	out->getCvMat(&resultVX);

	mGraph->removeNode(node);
}

void AppTableLookup::profiling(int n_times, int variant_numer)
{
	enum Target targets[APP_ONE_NODE];
	getVariantTarget(variant_numer, targets);
	Node *node = mGraph->addNode(mKernel_es[0], targets[0]);
	node->connect(3, in->getVxImage(), lut, out->getVxImage());
	if (!mGraph->verify())
		return;

	Node *nodes[] = { node };
	printProfilingResult(n_times, APP_ONE_NODE, nodes);
}

bool AppTableLookup::verify()
{
	Mat *resultGolden;
	Node *node = mGraph->addNode(mKernel_es[0], TARGET_C_MODEL);
	node->connect(3, in->getVxImage(), lut, out->getVxImage());
	if (mGraph->verify())
		mGraph->process();
	out->getCvMat(&resultGolden);
	mGraph->removeNode(node);

	return verifyTwoMat(*resultGolden, *resultVX);
}

void AppTableLookup::release()
{
	vx_status status = vxReleaseLUT(&lut);
	ERROR_CHECK(status);
	delete in;
	delete out;
	if (resultVX != NULL)
	{
		delete resultVX;
		resultVX = NULL;
	}
}

void AppTableLookup::releaseInput()
{
	src.release();
	delete [] lut_data;
}