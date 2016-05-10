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

#ifndef _VX_GRAPH_HPP_
#define _VX_GRAPH_HPP_

#define GRAPH_MAX_NODES           100
#define GRAPH_MAX_PARAMS          200
#define GRAPH_MAX_EPOCHS          20
#define GRAPH_MAX_NODES_PER_EPOCH 10

namespace OpenVX {

    class VX_CLASS Graph {

        friend class Context;
        friend class Node;

        Context*     m_pContext;

        vx_bool      m_verified;

        unsigned int m_epochCount;

        unsigned int m_nodeCount;
        Node*        m_apNodeList[GRAPH_MAX_NODES];   // TODO make all this stuff dynamic

        Node*        m_aapRunList[GRAPH_MAX_EPOCHS][GRAPH_MAX_NODES_PER_EPOCH];
        unsigned int m_aRunLength[GRAPH_MAX_EPOCHS];

        vx_status addNode(Node* pNode);
        void      setDirty();

    protected:
        vx_perf_t m_perf;

        Graph(Context* pContext);  // vxCreateGraph

    public:
        virtual ~Graph();          // vxReleaseGraph

        Context* context() { return m_pContext; }

        vx_status    verify();     // vxVerifyGraph
        vx_status    process();    // vxProcessGraph
        vx_status    schedule();   // vxScheduleGraph
        vx_status    wait();       // vxWaitGraph
        vx_status    addParameter(vx_parameter parameter);                                   // vxAddParameterToGraph
        vx_status    setParameterByIndex(vx_uint32 index, vx_enum dir, vx_reference value);  // vxSetGraphParameterByIndex
        vx_parameter getParameterByIndex(vx_uint32 index);                                   // vxGetGraphParameterByIndex
        vx_bool      isVerified();                                                           // vxIsGraphVerified

        // vxQueryGraph
        vx_perf_t* performance();
        vx_status  status();
        vx_uint32  numNodes();
        vx_uint32  numParameters();

        // Node *CreateNode(Kernel *k);  huh?

    };

};

#endif

