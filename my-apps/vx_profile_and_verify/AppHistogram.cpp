#include "Application.hpp"
#define NUM_OF_BIN 30

using namespace OpenVX;
using namespace cv;

AppHistogram::AppHistogram(Context &context)
	: Application(context, VX_KERNEL_HISTOGRAM)
{
}

AppHistogram::~AppHistogram()
{
}

void AppHistogram::prepareInput()
{
	src = imread(SRC_IMG_NAME1);
	NULLPTR_CHECK(src.data);
	resize(src, src, Size(IMG_WIDTH, IMG_HEIGHT));
	cvtColor(src, src, CV_RGB2GRAY);
}

void AppHistogram::setup()
{
	in = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8, src);
	
	vx_size numBins = NUM_OF_BIN;
	vx_int32 offset = 0;
	vx_uint32 range = 180;
	distribution = vxCreateDistribution(mContext.getVxContext(), numBins, offset, range);
	GET_STATUS_CHECK(distribution);
}

void AppHistogram::process(enum Target target_e)
{
	Node *node = mGraph->addNode(mKernel_e, target_e);
	node->connect(2, in->getVxImage(), distribution);
	if (mGraph->verify())
		mGraph->process();


	mGraph->removeNode(node);
}

bool AppHistogram::verify()
{
	vx_uint32 resultVX[NUM_OF_BIN], resultGolden[NUM_OF_BIN];

	vx_status status = vxCopyDistribution(distribution, resultVX, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
	ERROR_CHECK(status);

	Node *node = mGraph->addNode(mKernel_e, TARGET_C_MODEL);
	node->connect(2, in->getVxImage(), distribution);
	if (mGraph->verify())
		mGraph->process();
	status = vxCopyDistribution(distribution, resultGolden, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
	ERROR_CHECK(status);
	mGraph->removeNode(node);

	for (int cnt=0, i = 0; i < NUM_OF_BIN; i++)
	{
		if (resultVX[i] != resultGolden[i])
		{
			printf("fail at resultVX[%d]:%d != resultGolden[%d]:%d\n",
				i, resultVX[i], i, resultGolden[i]);
			if (++cnt == 10)
				return false;
		}
	}
	return true;
}

void AppHistogram::release()
{
	vx_status status = vxReleaseDistribution(&distribution);
	ERROR_CHECK(status);
	delete in;
}
