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
		Image *src1, *src2, *src3;
		VirtualImage *v023, *v1345, *v267, *v489, *v378, *v59;
		Image *dst6, *dst7, *dst8, *dst9;
	public:
		RandomCase1(Context &c);
		void prepareNodesAndDatas(Graph &graph, std::vector<vx_kernel_e> &kernel_es, std::vector<MyNode *> &nodes);
		void releaseDatas();
	};
}

#endif