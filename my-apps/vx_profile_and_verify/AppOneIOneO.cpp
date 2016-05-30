#include "Application.hpp"

using namespace OpenVX;
using namespace cv;

AppOneIOneO::AppOneIOneO(Context &context, vx_kernel_e kernel_e)
	: Application(context, APP_ONE_NODE, kernel_e), resultVX(NULL)
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
}

void AppOneIOneO::setup()
{
	in = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8, lena_src);
	out = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8);
}

void AppOneIOneO::process(int variant_numer)
{
	enum Target targets[APP_ONE_NODE];
	getVariantTarget(variant_numer, targets);
	Node *node = mGraph->addNode(mKernel_es[0], targets[0]);
	node->connect(2, in->getVxImage(), out->getVxImage());
	if (mGraph->verify())
		mGraph->process();

	out->getCvMat(&resultVX);

	mGraph->removeNode(node);
}

void AppOneIOneO::profiling(int n_times, int variant_numer)
{
	enum Target targets[APP_ONE_NODE];
	getVariantTarget(variant_numer, targets);
	Node *node = mGraph->addNode(mKernel_es[0], targets[0]);
	node->connect(2, in->getVxImage(), out->getVxImage());
	if (!mGraph->verify())
		return;
	
	Node *nodes[] = { node };
	printProfilingResult(n_times, APP_ONE_NODE, nodes, targets);
}

bool AppOneIOneO::verify()
{
	Mat *resultGolden;
	Node *node = mGraph->addNode(mKernel_es[0], TARGET_C_MODEL);
	node->connect(2, in->getVxImage(), out->getVxImage());
	if (mGraph->verify())
		mGraph->process();
	out->getCvMat(&resultGolden);
	mGraph->removeNode(node);

	bool result = verifyTwoMat(*resultGolden, *resultVX);
	delete resultGolden;
	return result;
}

void AppOneIOneO::release()
{
	delete in;
	delete out;
	if (resultVX != NULL)
	{
		delete resultVX;
		resultVX = NULL;
	}
}

void AppOneIOneO::releaseInput()
{
	lena_src.release();
}

void AppOneIOneO::generateApps(Context &context, std::vector<Application *> *apps)
{
	vx_kernel_e kernels[] = { VX_KERNEL_NOT, VX_KERNEL_BOX_3x3, VX_KERNEL_GAUSSIAN_3x3 };
	int n_kernels = sizeof(kernels) / sizeof(kernels[0]);
#if defined(EXPERIMENTAL_USE_HEXAGON) && defined(EXPERIMENTAL_USE_OPENCL)
	int node_index = 0, support_target = 3;
#elif defined(EXPERIMENTAL_USE_OPENCL)
	int node_index = 0, support_target = 2;
#else
	int node_index = 0, support_target = 1;
#endif

	for(int i=0; i<n_kernels; i++)
	{
		AppOneIOneO *app = new AppOneIOneO(context, kernels[i]);
#if defined(EXPERIMENTAL_USE_HEXAGON) && defined(EXPERIMENTAL_USE_OPENCL)
		app->setSupportTargets(node_index, support_target, TARGET_C_MODEL, TARGET_OPENCL, TARGET_HEXAGON);
#elif defined(EXPERIMENTAL_USE_OPENCL)
		app->setSupportTargets(node_index, support_target, TARGET_C_MODEL, TARGET_OPENCL);
#else
		app->setSupportTargets(node_index, support_target, TARGET_C_MODEL);
#endif
		apps->push_back(app);
	}
}