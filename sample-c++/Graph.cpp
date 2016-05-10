/*
 * Copyright (c) 2012-2014 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */

#include <vx.hpp>

using namespace OpenVX;

Graph::Graph(Context* pContext)
{
    m_pContext   = pContext;
    m_verified   = vx_false_e;
    m_epochCount = 0;
    m_nodeCount  = 0;
}

Graph::~Graph()
{
}

vx_status Graph::addNode(Node* pNode)
{
    if (m_verified)
    {
        // for now, return error and refuse addition. better: clear the graph and re-verify
        return VX_ERROR_NO_RESOURCES;
    }
    if (m_nodeCount < GRAPH_MAX_NODES)
    {
        m_apNodeList[m_nodeCount] = pNode;
        m_nodeCount++;
        return VX_SUCCESS;
    }
    else
    {
        return VX_ERROR_NO_RESOURCES;
    }
}

void Graph::setDirty()
{
    m_verified = vx_false_e;
}

vx_status Graph::verify()
{
    unsigned int nodeIndex, paramIndex;
    Node*        pNode;
    Parameter*   pParam;
    bool         success;

    // run through and init all nodes
    for (nodeIndex=0; nodeIndex<m_nodeCount; nodeIndex++) {
        pNode = m_apNodeList[nodeIndex];
        success = pNode->init();
        if (!success) {
            return VX_ERROR_INVALID_NODE;
        }
    }

    // mark all nodes as unscheduled
    for (nodeIndex=0; nodeIndex < m_nodeCount; nodeIndex++) {
        pNode = m_apNodeList[nodeIndex];
        pNode->m_scheduled = false;
    }

    // generate the run list: a list of lists of all nodes that can be run at each epoch

    unsigned int epoch = 0;
    unsigned int nodesScheduled = 0;
    memset(&(m_aRunLength[0]), 0, sizeof(m_aRunLength));

    while (nodesScheduled < m_nodeCount) {

        if (epoch == GRAPH_MAX_EPOCHS) {
            // either the graph is too darn big or it's just plain broken and unrunnable
            // TODO be smarter about distinguishing these (and other) different failure conditions
            return VX_ERROR_INVALID_GRAPH;
        }

        // find all nodes for which all inputs are available and schedule them for this epoch
        for (nodeIndex=0; nodeIndex < m_nodeCount; nodeIndex++) {
            pNode = m_apNodeList[nodeIndex];
            if (!pNode->m_scheduled) {
                bool runnable = true;  // assume runnable until proven otherwise
                for (paramIndex=0; paramIndex < pNode->m_paramsInCount; paramIndex++) {
                    pParam = pNode->m_apParamsIn[paramIndex];
                    runnable &= pParam->isValid();
                }
                if (runnable) {
                    m_aapRunList[epoch][m_aRunLength[epoch]] = pNode;
                    m_aRunLength[epoch]++;
                    pNode->m_scheduled = true;
                    nodesScheduled++;
                }
            }
        }

        if (!m_aRunLength[epoch]) {
            // failed to find any runnable nodes in this epoch. graph connectivity is suspect
            return VX_ERROR_INVALID_GRAPH;
        }

        // tag all parameters produced by the nodes in this epoch as now being available
        for (nodeIndex=0; nodeIndex < m_aRunLength[epoch]; nodeIndex++) {
            pNode = m_aapRunList[epoch][nodeIndex];
            for (paramIndex=0; paramIndex < pNode->m_paramsOutCount; paramIndex++) {
                pParam = pNode->m_apParamsOut[paramIndex];
                pParam->markValid();
            }
        }

        // move to the next epoch and repeat until all nodes are scheduled
        epoch++;
    }

    m_epochCount = epoch;

    m_verified = vx_true_e;
    return VX_SUCCESS;
}

vx_status Graph::process()
{
    unsigned int epoch, nodeIndex;

    if (!m_verified) {
        return VX_ERROR_INVALID_GRAPH;
    }

    // iterate through the run list, executing each node
    for (epoch=0; epoch<m_epochCount; epoch++) {
        for (nodeIndex=0; nodeIndex < m_aRunLength[epoch]; nodeIndex++) {
            Node* pNode = m_aapRunList[epoch][nodeIndex];
            vx_status status = pNode->execute(ttCPU);
            if (status != VX_SUCCESS) {
                return status;
            }
        }
    }

    return VX_SUCCESS;
}

vx_status Graph::schedule()
{
    return VX_ERROR_NOT_SUPPORTED;  // BUG BUG vxScheduleGraph(handle());
}

vx_status Graph::wait()
{
    return VX_ERROR_NOT_SUPPORTED;  // BUG BUG vxWaitGraph(handle());
}

vx_status Graph::addParameter(vx_parameter parameter)
{
    return VX_ERROR_NOT_SUPPORTED;  // BUG BUG vxAddParameterToGraph(handle(), parameter);
}

vx_status Graph::setParameterByIndex(vx_uint32 index, vx_enum dir, vx_reference value)
{
    return VX_ERROR_NOT_SUPPORTED;  // BUG BUG vxSetGraphParameterByIndex(handle(), index, dir, value);
}

vx_parameter Graph::getParameterByIndex(vx_uint32 index)
{
    return 0;  // BUG BUG vxGetGraphParameterByIndex(handle(), index);
}

vx_bool Graph::isVerified()
{
    return m_verified;
}

// vxQueryGraph
vx_perf_t* Graph::performance()
{
    return NULL;
//     vxQueryGraph(handle(), VX_GRAPH_PERFORMANCE, &m_perf, sizeof(m_perf));
//     return &m_perf;
}

vx_status Graph::status()
{
    return VX_ERROR_NOT_SUPPORTED;
//     vx_status v;
//     vxQueryGraph(handle(), VX_GRAPH_STATE, &v, sizeof(v));
//     return v;
}

vx_uint32 Graph::numNodes()
{
    return m_nodeCount;
}

vx_uint32 Graph::numParameters()
{
    return VX_ERROR_NOT_SUPPORTED;
//     vx_uint32 v;
//     vxQueryGraph(handle(), VX_GRAPH_NUMPARAMETERS, &v, sizeof(v));
//     return v;
}

