#include "Application.hpp"

using namespace OpenVX;
using namespace cv;

AppTwoIOneO::AppTwoIOneO(Context &context, vx_kernel_e kernel_e)
	: Application(context, APP_ONE_NODE, kernel_e), resultVX(NULL)
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
	enum Target targets[APP_ONE_NODE];
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
	enum Target targets[APP_ONE_NODE];
	getVariantTarget(variant_numer, targets);
	
	Node *node = mGraph->addNode(mKernel_es[0], targets[0]);
	node->connect(3, in1->getVxImage(), in2->getVxImage(), out->getVxImage());
	if (!mGraph->verify())
		return;

	Node *nodes[] = { node };
	printProfilingResult(n_times, APP_ONE_NODE, nodes, targets);
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

	bool result = verifyTwoMat(*resultGolden, *resultVX);
	delete resultGolden;
	return result;
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

void AppTwoIOneO::generateApps(Context &context, std::vector<Application *> *apps)
{
	vx_kernel_e kernels[] = { VX_KERNEL_AND, VX_KERNEL_XOR };
	int n_kernels = sizeof(kernels) / sizeof(kernels[0]);
#if defined(EXPERIMENTAL_USE_HEXAGON) && defined(EXPERIMENTAL_USE_OPENCL) && defined(EXPERIMENTAL_USE_FASTCV)
	int node_index = 0, support_target = 4;
#elif defined(EXPERIMENTAL_USE_HEXAGON) && defined(EXPERIMENTAL_USE_OPENCL)
	int node_index = 0, support_target = 3;
#elif defined(EXPERIMENTAL_USE_OPENCL)
	int node_index = 0, support_target = 2;
#else
	int node_index = 0, support_target = 1;
#endif

	for (int i = 0; i<n_kernels; i++)
	{
		AppTwoIOneO *app = new AppTwoIOneO(context, kernels[i]);
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
}