#ifndef _MY_NODE_H_
#define _MY_NODE_H_
#include "vx.hpp"
#include <vector>

namespace OpenVX
{
	class ProfileData;

	class Node;

	class DataObject;

	class MyNode
	{
	private:
		float mRank;
		const enum vx_kernel_e m_kernel_e;
		int m_input_p_cnt, m_output_p_cnt;
		std::vector<DataObject *> m_inputs, m_outputs;
		MyNode *mPropagateSuccessor;
		Node *mNode;
		bool visited;
		Target mTarget;
		bool inCluster;

		int getPropagateRank(std::vector<MyNode *> &nodes);

		float rank(std::vector<MyNode *> &nodes, ProfileData &profileData);

		static void addNodesToVxGraph(Graph& graph, std::vector<MyNode *> &nodes);
	public:
		MyNode(enum vx_kernel_e kernel_e);

		~MyNode();

		Node *getNode() const;

		void connect(int input_p_cnt, int output_p_cnt, ...);

		Target getTarget() const;

		vx_kernel_e getKernele() const;

		static bool addClusterIfValid(Target target, MyNode *node);

		static void generateNodes(int n_nodes,std::vector<MyNode *> &nodes, std::vector<vx_kernel_e> &kernel_es);

		static void priority(Target pTarget, Graph& graph, std::vector<vx_kernel_e> &kernel_es, std::vector<MyNode *> &nodes);

		static void nodeCoarsen(Graph& graph, ProfileData &profileData, std::vector<vx_kernel_e> &kernel_es, std::vector<MyNode *> &nodes);

		static void clusterToCSV(std::fstream &csvCluster, int n_nodes);

		static void releaseNodes(Graph& graph, std::vector<MyNode *> &nodes);
	};
}

#endif