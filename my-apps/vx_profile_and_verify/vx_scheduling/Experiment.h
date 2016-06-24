#ifndef _EXPERIMENT_H_
#define _EXPERIMENT_H_
#include "vx.hpp"
#include <vector>

namespace OpenVX
{
	class MyNode;
	class Context;
	class Graph;

	class Experiment
	{
	protected:
		Context &context;
	public:
		const char *name;
		Experiment(const char *n, Context &c);
		virtual ~Experiment() {}
		virtual void prepareNodesAndDatas(Graph &graph, std::vector<vx_kernel_e> &kernel_es, std::vector<MyNode *> &nodes) = 0;
		virtual void releaseDatas() = 0;
	};

	class RandomFaceDetection : public Experiment
	{
		Image *in, *out;
		VirtualImage *tmp01, *tmp123, *tmp24, *tmp35, *tmp46, *tmp56;
	public:
		RandomFaceDetection(Context &c);
		void prepareNodesAndDatas(Graph &graph, std::vector<vx_kernel_e> &kernel_es, std::vector<MyNode *> &nodes);
		void releaseDatas();
	};

	class SuperResolution : public Experiment
	{
		Image *src1, *src2, *result;
		VirtualImage *subOut, *thL, *thR;
		vx_scalar spolicy;
		vx_threshold mThreshold;
	public:
		SuperResolution(Context &c);
		void prepareNodesAndDatas(Graph &graph, std::vector<vx_kernel_e> &kernel_es, std::vector<MyNode *> &nodes);
		void releaseDatas();
	};

	class RandomCase1 : public Experiment
	{
		Image *src1, *src2;
		VirtualImage *v034, *v13456, *v256, *v37, *v47, *v58, *v68, *v79, *v89;
		Image *dst;
	public:
		RandomCase1(Context &c);
		void prepareNodesAndDatas(Graph &graph, std::vector<vx_kernel_e> &kernel_es, std::vector<MyNode *> &nodes);
		void releaseDatas();
	};

	class RandomCase2 : public Experiment
	{
		Image *src;
		VirtualImage *v01356, *v234, *v15, *v367, *v47;
		VirtualImage *v58, *v68, *v89, *v79;
		Image *dst;
	public:
		RandomCase2(Context &c);
		void prepareNodesAndDatas(Graph &graph, std::vector<vx_kernel_e> &kernel_es, std::vector<MyNode *> &nodes);
		void releaseDatas();
	};
}

#endif