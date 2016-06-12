#ifndef _GRAPH_HPP_
#define _GRAPH_HPP_
#include "vx.hpp"

namespace OpenVX 
{

	class Graph 
	{
		vx_graph m_graph;
		Context &m_ontext;

	public:
		Graph(Context &context);  // vxCreateGraph

		~Graph();          // vxReleaseGraph

		Node *addNode(vx_kernel_e kernel_e, enum Target target_e);

		void removeNode(Node *node);

		void setLocalOptimized(vx_reference ref);

		vx_graph getVxGraph() const;
		
		vx_perf_t getPerformance() const;

		Context &getContext() const;

		bool verify();     // vxVerifyGraph
		bool process();    // vxProcessGraph
	};

};

#endif