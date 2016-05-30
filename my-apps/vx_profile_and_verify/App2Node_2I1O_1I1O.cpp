#include "Application.hpp"
#include <iostream>
#include <vector>
using namespace OpenVX;
using namespace cv;

App2Node_2I1O_1I1O::App2Node_2I1O_1I1O(Context &context, vx_kernel_e kernel_e1, vx_kernel_e kernel_e2)
	: Application(context, APP_TWO_NODE, kernel_e1, kernel_e2), resultVX(NULL)
{
}

App2Node_2I1O_1I1O::~App2Node_2I1O_1I1O()
{
}

void App2Node_2I1O_1I1O::prepareInput()
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

void App2Node_2I1O_1I1O::setup()
{
	in1 = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8, src1);
	in2 = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8, src2);
	out = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8);
	tmp = new VirtualImage(*mGraph, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8);
}

void App2Node_2I1O_1I1O::process(int variant_numer)
{
	enum Target targets[APP_TWO_NODE];
	getVariantTarget(variant_numer, targets);
	Node *node1 = mGraph->addNode(mKernel_es[0], targets[0]);
	Node *node2 = mGraph->addNode(mKernel_es[1], targets[1]);
	node1->connect(3, in1->getVxImage(), in2->getVxImage(), tmp->getVxImage());
	node2->connect(2, tmp->getVxImage(), out->getVxImage());
	if (mGraph->verify())
		mGraph->process();

	out->getCvMat(&resultVX);

	mGraph->removeNode(node1);
	mGraph->removeNode(node2);
}

void App2Node_2I1O_1I1O::profiling(int n_times, int variant_numer)
{
	enum Target targets[APP_TWO_NODE];
	getVariantTarget(variant_numer, targets);
	Node *node1 = mGraph->addNode(mKernel_es[0], targets[0]);
	Node *node2 = mGraph->addNode(mKernel_es[1], targets[1]);
	node1->connect(3, in1->getVxImage(), in2->getVxImage(), tmp->getVxImage());
	node2->connect(2, tmp->getVxImage(), out->getVxImage());
	if (!mGraph->verify())
		return;
	
	Node *nodes[] = { node1, node2 };
	printProfilingResult(n_times, APP_TWO_NODE, nodes, targets);
}

bool App2Node_2I1O_1I1O::verify()
{
	Mat *resultGolden;
	Node *node1 = mGraph->addNode(mKernel_es[0], TARGET_C_MODEL);
	Node *node2 = mGraph->addNode(mKernel_es[1], TARGET_C_MODEL);
	node1->connect(3, in1->getVxImage(), in2->getVxImage(), tmp->getVxImage());
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

void App2Node_2I1O_1I1O::release()
{
	delete in1;
	delete in2;
	delete out;
	delete tmp;
	if (resultVX != NULL)
	{
		delete resultVX;
		resultVX = NULL;
	}
}

void App2Node_2I1O_1I1O::releaseInput()
{
	src1.release();
	src2.release();
}

void App2Node_2I1O_1I1O::generateApps(Context &context, std::vector<Application *> *apps)
{
	vx_kernel_e kernels_list1[] = { VX_KERNEL_AND, VX_KERNEL_XOR }; 
	vx_kernel_e kernels_list2[] = { VX_KERNEL_NOT, VX_KERNEL_BOX_3x3, VX_KERNEL_GAUSSIAN_3x3 };
	int app_index = 0;
	int n_kernels_list1 = sizeof(kernels_list1) / sizeof(kernels_list1[0]);
	int n_kernels_list2 = sizeof(kernels_list2) / sizeof(kernels_list2[0]);
#if defined(EXPERIMENTAL_USE_HEXAGON) && defined(EXPERIMENTAL_USE_OPENCL)
	int node_0 = 0, node_1 = 1, support_target = 3;
#elif defined(EXPERIMENTAL_USE_OPENCL)
	int node_0 = 0, node_1 = 1, support_target = 2;
#else
	int node_0 = 0, node_1 = 1, support_target = 1;
#endif

	for(int i=0; i<n_kernels_list1; i++)
	{
		for(int j=0; j<n_kernels_list2; j++, app_index++)
		{
			App2Node_2I1O_1I1O *app = new App2Node_2I1O_1I1O(context, kernels_list1[i], kernels_list2[j]);

#if defined(EXPERIMENTAL_USE_HEXAGON) && defined(EXPERIMENTAL_USE_OPENCL)
			app->setSupportTargets(node_0, support_target, TARGET_C_MODEL, TARGET_OPENCL, TARGET_HEXAGON);
			app->setSupportTargets(node_1, support_target, TARGET_C_MODEL, TARGET_OPENCL, TARGET_HEXAGON);
#elif defined(EXPERIMENTAL_USE_OPENCL)
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