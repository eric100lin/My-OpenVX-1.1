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

		int getPropagateRank(int n_nodes, MyNode **nodes);

		float rank(int n_nodes, MyNode **nodes, ProfileData &profileData);
	public:
		MyNode(enum vx_kernel_e kernel_e);

		~MyNode();

		Node *getNode() const;

		void connect(int input_p_cnt, int output_p_cnt, ...);

		Target getTarget() const;

		vx_kernel_e getKernele() const;

		static bool addClusterIfValid(Target target, MyNode *node);

		static void generateNodes(int n_nodes, MyNode **nodes, vx_kernel_e *kernel_es);

		static void nodeCoarsen(Graph& graph, ProfileData &profileData, int n_nodes, vx_kernel_e *kernel_es, MyNode **nodes);

		static void releaseNodes(Graph& graph, int n_nodes, MyNode **nodes);
	};
}

#endif