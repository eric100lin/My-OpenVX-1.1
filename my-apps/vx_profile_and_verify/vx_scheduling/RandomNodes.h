#ifndef _RANDOM_NODES_H_
#define _RANDOM_NODES_H_
#include "vx.hpp"
#include <vector>

namespace OpenVX
{
	class MyNode;
	class Context;

	class RandomNodes
	{
	public:
		virtual ~RandomNodes() {}
	};

	class OneIOneONodes : public RandomNodes
	{
	private:
		vx_threshold mThreshold;
	public:
		~OneIOneONodes();
		static vx_kernel_e random(Context &context, vx_image src, vx_image dst, std::vector<MyNode *> &nodes);
	};

	class TwoIOneONodes : public RandomNodes
	{
	private:
		vx_scalar spolicy;
	public:
		~TwoIOneONodes();
		static vx_kernel_e random(Context &context, vx_image src1, vx_image src2, vx_image dst, std::vector<MyNode *> &nodes);
	};
}

#endif