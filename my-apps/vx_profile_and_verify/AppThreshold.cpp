#include "Application.hpp"

using namespace OpenVX;
using namespace cv;

AppThreshold::AppThreshold(Context &context, int threshold)
	: Application(context, APP_ONE_NODE, VX_KERNEL_THRESHOLD), resultVX(NULL)
{
	vx_enum thresh_type = VX_THRESHOLD_TYPE_BINARY;
	vx_enum data_type = VX_TYPE_UINT8;
	mThreshold = vxCreateThreshold(context.getVxContext(), thresh_type, data_type);
	vx_int32 threshold_value = threshold;
	vxSetThresholdAttribute(mThreshold, VX_THRESHOLD_THRESHOLD_VALUE, &threshold_value, sizeof(vx_int32));
}

AppThreshold::~AppThreshold()
{
	vxReleaseThreshold(&mThreshold);
}

void AppThreshold::prepareInput()
{
	src = imread(SRC_IMG_NAME1);
	NULLPTR_CHECK(src.data);
	resize(src, src, Size(IMG_WIDTH, IMG_HEIGHT));
	cvtColor(src, src, CV_RGB2GRAY);
}

void AppThreshold::setup()
{
	in = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8, src);
	out = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8);
}

void AppThreshold::process(int variant_numer)
{
	enum Target targets[APP_ONE_NODE];
	getVariantTarget(variant_numer, targets);
	Node *node = mGraph->addNode(mKernel_es[0], targets[0]);
	node->connect(3, in->getVxImage(), mThreshold, out->getVxImage());
	if (mGraph->verify())
		mGraph->process();

	out->getCvMat(&resultVX);

	mGraph->removeNode(node);
}

void AppThreshold::profiling(int n_times, int variant_numer)
{
	enum Target targets[APP_ONE_NODE];
	getVariantTarget(variant_numer, targets);
	Node *node = mGraph->addNode(mKernel_es[0], targets[0]);
	node->connect(3, in->getVxImage(), mThreshold, out->getVxImage());
	if (!mGraph->verify())
		return;

	Node *nodes[] = { node };
	printProfilingResult(n_times, APP_ONE_NODE, nodes, targets);
}

bool AppThreshold::verify()
{
	Mat *resultGolden;
	Node *node = mGraph->addNode(mKernel_es[0], TARGET_C_MODEL);
	node->connect(3, in->getVxImage(), mThreshold, out->getVxImage());
	if (mGraph->verify())
		mGraph->process();
	out->getCvMat(&resultGolden);
	mGraph->removeNode(node);

	bool result = verifyTwoMat(*resultGolden, *resultVX);
	delete resultGolden;
	return result;
}

void AppThreshold::release()
{
	delete in;
	delete out;
	if (resultVX != NULL)
	{
		delete resultVX;
		resultVX = NULL;
	}
}

void AppThreshold::releaseInput()
{
	src.release();
}

void AppThreshold::generateApps(Context &context, std::vector<Application *> *apps)
{
#if defined(EXPERIMENTAL_USE_HEXAGON) && defined(EXPERIMENTAL_USE_OPENCL) && defined(EXPERIMENTAL_USE_FASTCV)
	int node_index = 0, support_target = 4;
#elif defined(EXPERIMENTAL_USE_HEXAGON) && defined(EXPERIMENTAL_USE_OPENCL)
	int node_index = 0, support_target = 3;
#elif defined(EXPERIMENTAL_USE_OPENCL)
	int node_index = 0, support_target = 2;
#else
	int node_index = 0, support_target = 1;
#endif

	int threshold = 128;
	AppThreshold *app = new AppThreshold(context, threshold);
#if defined(EXPERIMENTAL_USE_HEXAGON) && defined(EXPERIMENTAL_USE_OPENCL) && defined(EXPERIMENTAL_USE_FASTCV)
	app->setSupportTargets(node_index, support_target, TARGET_C_MODEL, TARGET_OPENCL, TARGET_HEXAGON, TARGET_FASTCV);
#elif defined(EXPERIMENTAL_USE_HEXAGON) && defined(EXPERIMENTAL_USE_OPENCL)
	app->setSupportTargets(node_index, support_target, TARGET_C_MODEL, TARGET_OPENCL, TARGET_HEXAGON);
#elif defined(EXPERIMENTAL_USE_OPENCL)
	app->setSupportTargets(node_index, support_target, TARGET_C_MODEL, TARGET_OPENCL);
#else
	app->setSupportTargets(node_index, support_target, TARGET_C_MODEL);
#endif
	apps->push_back(app);
}