#include "Application.hpp"
#define LUT_SIZE 256
using namespace OpenVX;
using namespace cv;

AppTableLookup::AppTableLookup(Context &context)
	: Application(context), mKernel_e(VX_KERNEL_TABLE_LOOKUP), resultVX(NULL)
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

	in = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8, src);
	out = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8);

	lut = vxCreateLUT(mContext.getVxContext(), VX_TYPE_UINT8, LUT_SIZE);
	GET_STATUS_CHECK(lut);

	uchar lut_data[LUT_SIZE];
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
	vx_status status = vxCopyLUT(lut, lut_data, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);
	ERROR_CHECK(status);
}

void AppTableLookup::process()
{
	Node *node = mGraph->addNode(mKernel_e, TARGET_OPENCL);
	node->connect(3, in->getVxImage(), lut, out->getVxImage());
	if (mGraph->verify())
		mGraph->process();

	out->getCvMat(&resultVX);

	mGraph->removeNode(node);
}

bool AppTableLookup::verify()
{
	Mat *resultGolden;
	Node *node = mGraph->addNode(mKernel_e, TARGET_C_MODEL);
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
		delete resultVX;
}