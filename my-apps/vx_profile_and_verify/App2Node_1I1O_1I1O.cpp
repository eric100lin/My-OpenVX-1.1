#include "Application.hpp"
#include <iostream>
#include <vector>
using namespace OpenVX;
using namespace cv;

App2Node_1I1O_1I1O::App2Node_1I1O_1I1O(Context &context, vx_kernel_e kernel_e1, vx_kernel_e kernel_e2)
	: Application(context, 2, kernel_e1, kernel_e2), resultVX(NULL)
{
}

App2Node_1I1O_1I1O::~App2Node_1I1O_1I1O()
{
}

void App2Node_1I1O_1I1O::prepareInput()
{
	lena_src = imread(SRC_IMG_NAME1);
	NULLPTR_CHECK(lena_src.data);
	resize(lena_src, lena_src, Size(IMG_WIDTH, IMG_HEIGHT));
	cvtColor(lena_src, lena_src, CV_RGB2GRAY);
}

void App2Node_1I1O_1I1O::setup()
{
	in = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8, lena_src);
	out = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8);
	tmp = new VirtualImage(*mGraph, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8);
}

void App2Node_1I1O_1I1O::process(int variant_numer)
{
	enum Target targets[2];
	getVariantTarget(variant_numer, targets);
	Node *node1 = mGraph->addNode(mKernel_es[0], targets[0]);
	Node *node2 = mGraph->addNode(mKernel_es[1], targets[1]);
	node1->connect(2, in->getVxImage(), tmp->getVxImage());
	node2->connect(2, tmp->getVxImage(), out->getVxImage());
	if (mGraph->verify())
		mGraph->process();

	out->getCvMat(&resultVX);

	mGraph->removeNode(node1);
	mGraph->removeNode(node2);
}

void App2Node_1I1O_1I1O::profiling(int n_times, int variant_numer)
{
	enum Target targets[2];
	getVariantTarget(variant_numer, targets);
	Node *node1 = mGraph->addNode(mKernel_es[0], targets[0]);
	Node *node2 = mGraph->addNode(mKernel_es[1], targets[1]);
	node1->connect(2, in->getVxImage(), tmp->getVxImage());
	node2->connect(2, tmp->getVxImage(), out->getVxImage());
	if (!mGraph->verify())
		return;
	
	Node *nodes[] = { node1, node2 };
	printProfilingResult(n_times, 2, nodes);
}

bool App2Node_1I1O_1I1O::verify()
{
	Mat *resultGolden;
	Node *node1 = mGraph->addNode(mKernel_es[0], TARGET_C_MODEL);
	Node *node2 = mGraph->addNode(mKernel_es[1], TARGET_C_MODEL);
	node1->connect(2, in->getVxImage(), tmp->getVxImage());
	node2->connect(2, tmp->getVxImage(), out->getVxImage());
	if (mGraph->verify())
		mGraph->process();
	out->getCvMat(&resultGolden);
	mGraph->removeNode(node1);
	mGraph->removeNode(node2);

	bool result = verifyTwoMat(*resultGolden, *resultVX);
	delete resultGolden;
	return result;
}

void App2Node_1I1O_1I1O::release()
{
	delete in;
	delete out;
	delete tmp;
	if (resultVX != NULL)
	{
		delete resultVX;
		resultVX = NULL;
	}
}

void App2Node_1I1O_1I1O::releaseInput()
{
	lena_src.release();
}

void App2Node_1I1O_1I1O::generateApps(Context &context, std::vector<Application *> *apps)
{
	vx_kernel_e kernels[] = { VX_KERNEL_NOT, VX_KERNEL_BOX_3x3, VX_KERNEL_GAUSSIAN_3x3 };
	int app_index = 0, n_kernels = sizeof(kernels) / sizeof(kernels[0]);
#ifdef EXPERIMENTAL_USE_HEXAGON	&& EXPERIMENTAL_USE_OPENCL
	int node_0 = 0, node_1 = 1, support_target = 3;
#elif EXPERIMENTAL_USE_OPENCL
	int node_0 = 0, node_1 = 1, support_target = 2;
#else
	int node_0 = 0, node_1 = 1, support_target = 1;
#endif

	for(int i=0; i<n_kernels; i++)
	{
		for(int j=0; j<n_kernels; j++, app_index++)
		{
			App2Node_1I1O_1I1O *app = new App2Node_1I1O_1I1O(context, kernels[i], kernels[j]);

#ifdef EXPERIMENTAL_USE_HEXAGON	&& EXPERIMENTAL_USE_OPENCL
			app->setSupportTargets(node_0, support_target, TARGET_C_MODEL, TARGET_OPENCL, TARGET_HEXAGON);
			app->setSupportTargets(node_1, support_target, TARGET_C_MODEL, TARGET_OPENCL, TARGET_HEXAGON);
#elif EXPERIMENTAL_USE_OPENCL
			app->setSupportTargets(node_0, support_target, TARGET_C_MODEL, TARGET_OPENCL);
			app->setSupportTargets(node_1, support_target, TARGET_C_MODEL, TARGET_OPENCL);
#else
			app->setSupportTargets(node_0, support_target, TARGET_C_MODEL);
			app->setSupportTargets(node_1, support_target, TARGET_C_MODEL);
#endif
			apps->push_back(app);
		}
	}
}