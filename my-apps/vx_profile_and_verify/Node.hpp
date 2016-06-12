#ifndef _NODE_HPP_
#define _NODE_HPP_
#include "vx.hpp"

namespace OpenVX
{

	class Node
	{
		vx_node m_node;
		Kernel *mKernel;

	public:
		Node(Graph &graph, vx_kernel_e kernel_e, enum Target target_e);  // vxCreateGenericNode

		~Node();

		vx_node getVxNode() const;
		
		vx_perf_t getPerformance() const;
		
		vx_perf_t getComputationTime() const;

		void connect(int n, ...);

		void setParameterByIndex(int idx, vx_reference ref);
	};

};

#endif