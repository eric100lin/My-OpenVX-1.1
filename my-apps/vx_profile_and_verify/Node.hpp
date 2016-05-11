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

		void connect(int n, ...);
	};

};

#endif