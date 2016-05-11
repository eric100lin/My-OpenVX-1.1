#include "Graph.hpp"

using namespace OpenVX;

Graph::Graph(Context &rContext) : m_ontext(rContext)
{
	m_graph = vxCreateGraph(rContext.getVxContext());
	GET_STATUS_CHECK(m_graph);
}

Graph::~Graph()
{
	vx_status status = vxReleaseGraph(&m_graph);
	ERROR_CHECK(status);
}

Node *Graph::addNode(vx_kernel_e kernel_e, enum Target target_e)
{
	return new Node(*this, kernel_e, target_e);
}

void Graph::removeNode(Node *node)
{
	vx_node vxNode = node->getVxNode();
	vx_status status = vxRemoveNode(&vxNode);
	ERROR_CHECK(status);

	delete node;
}

vx_graph Graph::getVxGraph() const
{
	return m_graph;
}

Context &Graph::getContext() const
{
	return m_ontext;
}

bool Graph::verify()
{
	vx_status status = vxVerifyGraph(m_graph);
	ERROR_CHECK(status);
	return (status == VX_SUCCESS);
}

bool Graph::process()
{
	vx_status status = vxProcessGraph(m_graph);
	ERROR_CHECK(status);
	return (status == VX_SUCCESS);
}
