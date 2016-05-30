#include "Application.hpp"
#include <iostream>
#include <vector>
using namespace OpenVX;
using namespace cv;

App3Node_1I1O_1I1O_2I1O::App3Node_1I1O_1I1O_2I1O(Context &context, 
	vx_kernel_e kernel_e1, vx_kernel_e kernel_e2, vx_kernel_e kernel_e3)
	: Application(context, APP_THREE_NODE, kernel_e1, kernel_e2, kernel_e3), resultVX(NULL)
{
}

App3Node_1I1O_1I1O_2I1O::~App3Node_1I1O_1I1O_2I1O()
{
}

void App3Node_1I1O_1I1O_2I1O::prepareInput()
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

void App3Node_1I1O_1I1O_2I1O::setup()
{
	in1 = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8, src1);
	in2 = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8, src2);
	out = new Image(mContext, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8);
	tmp1 = new VirtualImage(*mGraph, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8);
	tmp2 = new VirtualImage(*mGraph, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8);
}

void App3Node_1I1O_1I1O_2I1O::process(int variant_numer)
{
	enum Target targets[APP_THREE_NODE];
	getVariantTarget(variant_numer, targets);
	Node *node1 = mGraph->addNode(mKernel_es[0], targets[0]);
	Node *node2 = mGraph->addNode(mKernel_es[1], targets[1]);
	Node *node3 = mGraph->addNode(mKernel_es[2], targets[2]);
	node1->connect(2, in1->getVxImage(), tmp1->getVxImage());
	node2->connect(2, in2->getVxImage(), tmp2->getVxImage());
	node3->connect(3, tmp1->getVxImage(), tmp2->getVxImage(), out->getVxImage());
	if (mGraph->verify())
		mGraph->process();

	out->getCvMat(&resultVX);

	mGraph->removeNode(node1);
	mGraph->removeNode(node2);
	mGraph->removeNode(node3);
}

void App3Node_1I1O_1I1O_2I1O::profiling(int n_times, int variant_numer)
{
	enum Target targets[APP_THREE_NODE];
	getVariantTarget(variant_numer, targets);
	Node *node1 = mGraph->addNode(mKernel_es[0], targets[0]);
	Node *node2 = mGraph->addNode(mKernel_es[1], targets[1]);
	Node *node3 = mGraph->addNode(mKernel_es[2], targets[2]);
	node1->connect(2, in1->getVxImage(), tmp1->getVxImage());
	node2->connect(2, in2->getVxImage(), tmp2->getVxImage());
	node3->connect(3, tmp1->getVxImage(), tmp2->getVxImage(), out->getVxImage());
	if (mGraph->verify())
		mGraph->process();
	
	Node *nodes[] = { node1, node2, node3 };
	printProfilingResult(n_times, APP_THREE_NODE, nodes, targets);
}

bool App3Node_1I1O_1I1O_2I1O::verify()
{
	Mat *resultGolden;
	Node *node1 = mGraph->addNode(mKernel_es[0], TARGET_C_MODEL);
	Node *node2 = mGraph->addNode(mKernel_es[1], TARGET_C_MODEL);
	Node *node3 = mGraph->addNode(mKernel_es[2], TARGET_C_MODEL);
	node1->connect(2, in1->getVxImage(), tmp1->getVxImage());
	node2->connect(2, in2->getVxImage(), tmp2->getVxImage());
	node3->connect(3, tmp1->getVxImage(), tmp2->getVxImage(), out->getVxImage());
	if (mGraph->verify())
		mGraph->process();
	out->getCvMat(&resultGolden);
	mGraph->removeNode(node1);
	mGraph->removeNode(node2);
	mGraph->removeNode(node3);

	bool result = verifyTwoMat(*resultGolden, *resultVX);
	delete resultGolden;
	return result;
}

void App3Node_1I1O_1I1O_2I1O::release()
{
	delete in1;
	delete in2;
	delete out;
	delete tmp1;
	delete tmp2;
	if (resultVX != NULL)
	{
		delete resultVX;
		resultVX = NULL;
	}
}

void App3Node_1I1O_1I1O_2I1O::releaseInput()
{
	src1.release();
	src2.release();
}

void App3Node_1I1O_1I1O_2I1O::generateApps(Context &context, std::vector<Application *> *apps)
{
	vx_kernel_e kernel_2I1O_list[] = { VX_KERNEL_AND, VX_KERNEL_XOR }; 
	vx_kernel_e kernel_1I1O_list[] = { VX_KERNEL_NOT, VX_KERNEL_BOX_3x3, VX_KERNEL_GAUSSIAN_3x3 };
	int app_index = 0;
	int n_kernels_2I1O = sizeof(kernel_2I1O_list) / sizeof(kernel_2I1O_list[0]);
	int n_kernels_1I1O = sizeof(kernel_1I1O_list) / sizeof(kernel_1I1O_list[0]);
	int node_0 = 0, node_1 = 1, node_2 = 2;
#if defined(EXPERIMENTAL_USE_HEXAGON) && defined(EXPERIMENTAL_USE_OPENCL)
	int support_target = 3;
#elif defined(EXPERIMENTAL_USE_OPENCL)
	int support_target = 2;
#else
	int support_target = 1;
#endif

	for(int left_up=0; left_up<n_kernels_1I1O; left_up++)
	{
		for (int right_up = 0; right_up < n_kernels_1I1O; right_up++)
		{
			for (int bottom = 0; bottom < n_kernels_2I1O; bottom++, app_index++)
			{
				App3Node_1I1O_1I1O_2I1O *app = new App3Node_1I1O_1I1O_2I1O(context,
					kernel_1I1O_list[left_up], kernel_1I1O_list[right_up], kernel_1I1O_list[bottom]);

#if defined(EXPERIMENTAL_USE_HEXAGON) && defined(EXPERIMENTAL_USE_OPENCL)
				app->setSupportTargets(node_0, support_target, TARGET_C_MODEL, TARGET_OPENCL, TARGET_HEXAGON);
				app->setSupportTargets(node_1, support_target, TARGET_C_MODEL, TARGET_OPENCL, TARGET_HEXAGON);
				app->setSupportTargets(node_2, support_target, TARGET_C_MODEL, TARGET_OPENCL, TARGET_HEXAGON);
#elif defined(EXPERIMENTAL_USE_OPENCL)
				app->setSupportTargets(node_0, support_target, TARGET_C_MODEL, TARGET_OPENCL);
				app->setSupportTargets(node_1, support_target, TARGET_C_MODEL, TARGET_OPENCL);
				app->setSupportTargets(node_2, support_target, TARGET_C_MODEL, TARGET_OPENCL);
#else
				app->setSupportTargets(node_0, support_target, TARGET_C_MODEL);
				app->setSupportTargets(node_1, support_target, TARGET_C_MODEL);
				app->setSupportTargets(node_2, support_target, TARGET_C_MODEL);
#endif
				apps->push_back(app);
			}
		}
	}
}