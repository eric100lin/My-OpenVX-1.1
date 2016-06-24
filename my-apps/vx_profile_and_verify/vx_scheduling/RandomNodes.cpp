#include "RandomNodes.h"
#include "MyNode.h"
#include <cstdlib>
using namespace OpenVX;

static vx_kernel_e k_1I1Os[] = { VX_KERNEL_NOT, VX_KERNEL_BOX_3x3, VX_KERNEL_GAUSSIAN_3x3, VX_KERNEL_THRESHOLD };
static int n_1I1O = sizeof(k_1I1Os) / sizeof(k_1I1Os[0]);

vx_kernel_e OneIOneONodes::get(vx_kernel_e k, Context &context, vx_image src, vx_image dst, vector<MyNode *> &nodes)
{
	//int ki = rand() % n_1I1O;
	MyNode *node = new MyNode(k);
	if (k == VX_KERNEL_THRESHOLD)
	{
		OneIOneONodes *ptr = new OneIOneONodes();
		vx_enum thresh_type = VX_THRESHOLD_TYPE_BINARY;
		vx_enum data_type = VX_TYPE_UINT8;
		ptr->mThreshold = vxCreateThreshold(context.getVxContext(), thresh_type, data_type);
		node->setRandomNodesPoiter(ptr);
		node->connect(2, 1, src, ptr->mThreshold, dst);
	}
	else
	{
		node->connect(1, 1, src, dst);
	}
	nodes.push_back(node);
	return k;
}

OneIOneONodes::~OneIOneONodes()
{
	vxReleaseThreshold(&mThreshold);
}

static vx_kernel_e k_2I1Os[] = { VX_KERNEL_AND, VX_KERNEL_XOR, VX_KERNEL_ADD, VX_KERNEL_SUBTRACT };
static int n_2I1O = sizeof(k_2I1Os) / sizeof(k_2I1Os[0]);

vx_kernel_e TwoIOneONodes::get(vx_kernel_e k, Context &context, vx_image src1, vx_image src2, vx_image dst, vector<MyNode *> &nodes)
{
	//int ki = rand() % n_2I1O;
	MyNode *node = new MyNode(k);
	if (k == VX_KERNEL_ADD || k == VX_KERNEL_SUBTRACT)
	{
		TwoIOneONodes *ptr = new TwoIOneONodes();
		vx_enum policy = VX_CONVERT_POLICY_WRAP;
		ptr->spolicy = vxCreateScalar(context.getVxContext(), VX_TYPE_ENUM, &policy);
		node->setRandomNodesPoiter(ptr);
		node->connect(3, 1, src1, src2, ptr->spolicy, dst);
	}
	else
	{
		node->connect(2, 1, src1, src2, dst);
	}
	nodes.push_back(node);
	return k;
}

TwoIOneONodes::~TwoIOneONodes()
{
	vxReleaseScalar(&spolicy);
}