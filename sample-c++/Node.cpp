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
#include <c_model.h>
#include <debug_k.h>
#include <extras_k.h>

#ifndef dimof
#define dimof(x) (sizeof(x)/sizeof(x[0]))
#endif

using namespace OpenVX;

// BUGBUG TODO move all destinations to be the first parameter to match the Everything Else on Earth
// (and so optional inputs don't end up being after the output, which is dopey)

Node::Node(Graph* pGraph, BorderMode* pBorderMode)
{
    m_pGraph      = pGraph;
    m_pBorderMode = new BorderMode(pGraph->context(), pBorderMode);

    m_dirty       = true;
    m_scheduled   = false;

    m_paramsInCount = 0;
    m_paramsOutCount = 0;
    memset (m_apParams,    0, sizeof(m_apParams));
    memset (m_apParamsIn,  0, sizeof(m_apParamsIn));
    memset (m_apParamsOut, 0, sizeof(m_apParamsOut));

    // allocate empty ports for all possible properties
    for (int i=0; i<NODE_MAX_PARAMS; i++) {
        m_properties.m_apPortList[i] = new Port();
    }

    // add the node to the graph
    pGraph->addNode(this);
}

// TODO: when do nodes actually get deleted? who's allowed to do so?
// can the user just delete a node at random and expect the graph manager to deal with it?
Node::~Node()
{
    delete m_pBorderMode;
    for (int i=0; i<NODE_MAX_PARAMS; i++) {
        delete (m_properties.m_apPortList[i]);
    }
}

bool Node::init()
{
    if (!m_dirty) {
        // nothing to do
        return true;
    }

    // call the derived node's definition function to set up all node properties
    vx_status status = defineNode(&m_properties);
    if (status != VX_SUCCESS) {
        return false;
    }

    // sort parameters into in-going, out-going, and exhaustive lists
    m_paramsInCount = 0;
    m_paramsOutCount = 0;

    for (unsigned int portIndex=0; portIndex<m_properties.m_portCount; portIndex++) {
        Port* pPort = m_properties.m_apPortList[portIndex];
        if (pPort->direction() == pdIn) {
            m_apParamsIn[m_paramsInCount] = pPort->m_pParameter;
            m_paramsInCount++;
        }
        else if (pPort->direction() == pdOut) {
            m_apParamsOut[m_paramsOutCount] = pPort->m_pParameter;
            m_paramsOutCount++;
        }
        m_apParams[portIndex] = pPort->m_pParameter;
    }

    m_dirty = false;
    return true;
}

vx_status Node::execute(TargetType targetType)
{
    for (int i=0; i<VX_MAX_KERNEL_COUNT; i++) {
        if (m_properties.m_kernels[i].target == targetType) {
            // found the one we want. run it and return success
            KernelFunction kf = m_properties.m_kernels[i].kernel;
            return kf(m_properties.m_pSubGraph, m_apParams, (vx_uint32)m_properties.m_portCount);
        }
    }

    // no kernel available for this target
    return VX_ERROR_INVALID_NODE;
}

void Node::setDirty()
{
    m_dirty = true;
    m_pGraph->setDirty();
}

// setters for dynamic properties
vx_status Node::setLocalData(void* pData)
{
    m_pLocalData = pData;
    return VX_SUCCESS;
}

// getters
vx_char* Node::name()
{
    return m_properties.m_nodeName;
}

vx_status Node::status()
{
    return m_status;
}

vx_perf_t* Node::perf()
{
    return &m_perf;
}

//----------------------------------------------------------------------------
// derived nodes
// TODO split these out into more files
//----------------------------------------------------------------------------

// AbsDiff -------------------------------------------------------------------

static vx_status vxAbsDiffKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 3) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image in1 = (vx_image)ppParamList[0]->handle();
    vx_image in2 = (vx_image)ppParamList[1]->handle();
    vx_image out = (vx_image)ppParamList[2]->handle();

    return vxAbsDiff(in1, in2, out);
}

AbsDiffNode::AbsDiffNode(Graph* pGraph, Image* pSrcImage1, Image* pSrcImage2, Image* pDstImage) :
             Node(pGraph)
{
    // store the parameters
    m_pSrcImage1 = pSrcImage1;
    m_pSrcImage2 = pSrcImage2;
    m_pDstImage  = pDstImage;
}

vx_status AbsDiffNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.AbsDiff");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxAbsDiffKernelCPP;

    pProperties->m_portCount = 3;
    pProperties->m_apPortList[0]->set(m_pSrcImage1, pdIn);
    pProperties->m_apPortList[1]->set(m_pSrcImage2, pdIn);
    pProperties->m_apPortList[2]->set(m_pDstImage,  pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// Accumulate ----------------------------------------------------------------

static vx_status vxAccumulateKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 2) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image input = (vx_image)ppParamList[0]->handle();
    vx_image accum = (vx_image)ppParamList[1]->handle();

    return vxAccumulate(input, accum);
}

AccumulateNode::AccumulateNode(Graph* pGraph, Image* pSrcImage, Image* pAccum) :
                Node(pGraph)
{
    // store the parameters
    m_pSrcImage = pSrcImage;
    m_pAccum    = pAccum;
}

vx_status AccumulateNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.accumulate");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxAccumulateKernelCPP;

    pProperties->m_portCount = 2;
    pProperties->m_apPortList[0]->set(m_pSrcImage, pdIn);
    pProperties->m_apPortList[1]->set(m_pAccum,    pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// AccumulateWeighted --------------------------------------------------------

static vx_status vxAccumulateWeightedKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 3) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image  input  = (vx_image )ppParamList[0]->handle();
    vx_scalar weight = (vx_scalar)ppParamList[1]->handle();
    vx_image  accum  = (vx_image )ppParamList[2]->handle();

    return vxAccumulateWeighted(input, weight, accum);
}

AccumulateWeightedNode::AccumulateWeightedNode(Graph* pGraph, Image* pSrcImage, Float32* pWeight, Image* pAccum) :
                        Node(pGraph)
{
    // store the parameters
    m_pSrcImage = pSrcImage;
    m_pWeight   = pWeight;
    m_pAccum    = pAccum;
}

vx_status AccumulateWeightedNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.accumulateweighted");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxAccumulateWeightedKernelCPP;

    pProperties->m_portCount = 3;
    pProperties->m_apPortList[0]->set(m_pSrcImage, pdIn);
    pProperties->m_apPortList[1]->set(m_pWeight,   pdIn);
    pProperties->m_apPortList[2]->set(m_pAccum,    pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// AccumulateSquare ----------------------------------------------------------

static vx_status vxAccumulateSquareKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 3) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image  input  = (vx_image )ppParamList[0]->handle();
    vx_scalar weight = (vx_scalar)ppParamList[1]->handle();
    vx_image  accum  = (vx_image )ppParamList[2]->handle();

    return vxAccumulateSquare(input, weight, accum);
}

AccumulateSquareNode::AccumulateSquareNode(Graph* pGraph, Image* pSrcImage, Float32* pWeight, Image* pAccum) :
                      Node(pGraph)
{
    // store the parameters
    m_pSrcImage = pSrcImage;
    m_pWeight   = pWeight;
    m_pAccum    = pAccum;
}

vx_status AccumulateSquareNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.accumulatesquare");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxAccumulateSquareKernelCPP;

    pProperties->m_portCount = 3;
    pProperties->m_apPortList[0]->set(m_pSrcImage, pdIn);
    pProperties->m_apPortList[1]->set(m_pWeight,   pdIn);
    pProperties->m_apPortList[2]->set(m_pAccum,    pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// Addition ------------------------------------------------------------------

static vx_status vxAdditionKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 4) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image  in0    = (vx_image )ppParamList[0]->handle();
    vx_image  in1    = (vx_image )ppParamList[1]->handle();
    vx_scalar policy = (vx_scalar)ppParamList[2]->handle();
    vx_image  output = (vx_image )ppParamList[3]->handle();

    return vxAddition(in0, in1, policy, output);
}

AdditionNode::AdditionNode(Graph* pGraph, Image* pSrcImage1, Image* pSrcImage2, Enumerant* pPolicy, Image* pDstImage) :
              Node(pGraph)
{
    // store the parameters
    m_pSrcImage1 = pSrcImage1;
    m_pSrcImage2 = pSrcImage2;
    m_pPolicy    = pPolicy;
    m_pDstImage  = pDstImage;
}

vx_status AdditionNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.addition");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxAdditionKernelCPP;

    pProperties->m_portCount = 4;
    pProperties->m_apPortList[0]->set(m_pSrcImage1, pdIn);
    pProperties->m_apPortList[1]->set(m_pSrcImage2, pdIn);
    pProperties->m_apPortList[2]->set(m_pPolicy,    pdIn);
    pProperties->m_apPortList[3]->set(m_pDstImage,  pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// Subtraction ------------------------------------------------------------------

static vx_status vxSubtractionKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 4) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image  in0    = (vx_image )ppParamList[0]->handle();
    vx_image  in1    = (vx_image )ppParamList[1]->handle();
    vx_scalar policy = (vx_scalar)ppParamList[2]->handle();
    vx_image  output = (vx_image )ppParamList[3]->handle();

    return vxSubtraction(in0, in1, policy, output);
}

SubtractionNode::SubtractionNode(Graph* pGraph, Image* pSrcImage1, Image* pSrcImage2, Enumerant* pPolicy, Image* pDstImage) :
                 Node(pGraph)
{
    // store the parameters
    m_pSrcImage1 = pSrcImage1;
    m_pSrcImage2 = pSrcImage2;
    m_pPolicy    = pPolicy;
    m_pDstImage  = pDstImage;
}

vx_status SubtractionNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.subtraction");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxSubtractionKernelCPP;

    pProperties->m_portCount = 4;
    pProperties->m_apPortList[0]->set(m_pSrcImage1, pdIn);
    pProperties->m_apPortList[1]->set(m_pSrcImage2, pdIn);
    pProperties->m_apPortList[2]->set(m_pPolicy,    pdIn);
    pProperties->m_apPortList[3]->set(m_pDstImage,  pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// And -----------------------------------------------------------------------

static vx_status vxAndKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 3) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image in1    = (vx_image)ppParamList[0]->handle();
    vx_image in2    = (vx_image)ppParamList[1]->handle();
    vx_image output = (vx_image)ppParamList[2]->handle();

    return vxAnd(in1, in2, output);
}

AndNode::AndNode(Graph* pGraph, Image* pSrcImage1, Image* pSrcImage2, Image* pDstImage) :
         Node(pGraph)
{
    // store the parameters
    m_pSrcImage1 = pSrcImage1;
    m_pSrcImage2 = pSrcImage2;
    m_pDstImage  = pDstImage;
}

vx_status AndNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.and");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxAndKernelCPP;

    pProperties->m_portCount = 3;
    pProperties->m_apPortList[0]->set(m_pSrcImage1, pdIn);
    pProperties->m_apPortList[1]->set(m_pSrcImage2, pdIn);
    pProperties->m_apPortList[2]->set(m_pDstImage,  pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// Or ------------------------------------------------------------------------

static vx_status vxOrKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 3) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image in1    = (vx_image)ppParamList[0]->handle();
    vx_image in2    = (vx_image)ppParamList[1]->handle();
    vx_image output = (vx_image)ppParamList[2]->handle();

    return vxOr(in1, in2, output);
}

OrNode::OrNode(Graph* pGraph, Image* pSrcImage1, Image* pSrcImage2, Image* pDstImage) :
        Node(pGraph)
{
    // store the parameters
    m_pSrcImage1 = pSrcImage1;
    m_pSrcImage2 = pSrcImage2;
    m_pDstImage  = pDstImage;
}

vx_status OrNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.or");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxOrKernelCPP;

    pProperties->m_portCount = 3;
    pProperties->m_apPortList[0]->set(m_pSrcImage1, pdIn);
    pProperties->m_apPortList[1]->set(m_pSrcImage2, pdIn);
    pProperties->m_apPortList[2]->set(m_pDstImage,  pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// Xor -----------------------------------------------------------------------

static vx_status vxXorKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 3) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image in1    = (vx_image)ppParamList[0]->handle();
    vx_image in2    = (vx_image)ppParamList[1]->handle();
    vx_image output = (vx_image)ppParamList[2]->handle();

    return vxXor(in1, in2, output);
}

XorNode::XorNode(Graph* pGraph, Image* pSrcImage1, Image* pSrcImage2, Image* pDstImage) :
         Node(pGraph)
{
    // store the parameters
    m_pSrcImage1 = pSrcImage1;
    m_pSrcImage2 = pSrcImage2;
    m_pDstImage  = pDstImage;
}

vx_status XorNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.xor");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxXorKernelCPP;

    pProperties->m_portCount = 3;
    pProperties->m_apPortList[0]->set(m_pSrcImage1, pdIn);
    pProperties->m_apPortList[1]->set(m_pSrcImage2, pdIn);
    pProperties->m_apPortList[2]->set(m_pDstImage,  pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// Not -----------------------------------------------------------------------

static vx_status vxNotKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 2) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image input  = (vx_image)ppParamList[0]->handle();
    vx_image output = (vx_image)ppParamList[1]->handle();

    return vxNot(input, output);
}

NotNode::NotNode(Graph* pGraph, Image* pSrcImage, Image* pDstImage) :
         Node(pGraph)
{
    // store the parameters
    m_pSrcImage = pSrcImage;
    m_pDstImage = pDstImage;
}

vx_status NotNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.not");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxNotKernelCPP;

    pProperties->m_portCount = 2;
    pProperties->m_apPortList[0]->set(m_pSrcImage, pdIn);
    pProperties->m_apPortList[1]->set(m_pDstImage, pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// Canny Edge Detector -------------------------------------------------------

static vx_status vxCannyEdgeKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    // i think the parameter list is pointless here, because we've already baked them into the sub-graph in "defineNode".
    // admittedly, this is rather heavy-weight in that the graph must be re-verified when params change. lighter weight solution
    // migh be to overload the "process" function with a second version that takes parameters that are mapped a list of exported graph params
    if (pSubGraph == NULL) {
        return VX_ERROR_INVALID_PARAMETERS;
    }
    return pSubGraph->process();
}

CannyEdgeDetectorNode::CannyEdgeDetectorNode(Graph* pGraph, Image* pSrcImage, Image* pDstImage, Threshold<vx_uint8>* pThreshold) :
                       Node(pGraph)
{
    // store the parameters
    m_pSrcImage  = pSrcImage;
    m_pDstImage  = pDstImage;
    m_pThreshold = pThreshold;
}

vx_status CannyEdgeDetectorNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.cannyedgedetector");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxCannyEdgeKernelCPP;

    pProperties->m_portCount = 2;
    pProperties->m_apPortList[0]->set(m_pSrcImage, pdIn);
    pProperties->m_apPortList[1]->set(m_pDstImage, pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    // generate the sub-graph ---------------------------

    // inherit the context from the graph of which this node is a part
    Context* pContext = graph()->context();

    // create our own new sub-graph
    pProperties->m_pSubGraph = pContext->createGraph();

    if (!pProperties->m_pSubGraph) {
        return VX_ERROR_NO_RESOURCES;
    }

    vx_int16 gx[3][3] = {
        { 0, 0, 0},
        {-1, 0, 1},
        { 0, 0, 0},
    };
    vx_int16 gy[3][3] = {
        { 0,-1, 0},
        { 0, 0, 0},
        { 0, 1, 0},
    };

    m_pConv[0] = pContext->createConvolution(3, 3);
    m_pConv[0]->accessCoefficients(NULL);
    m_pConv[0]->commitCoefficients((vx_int16*)gx);
    m_pConv[1] = pContext->createConvolution(3, 3);
    m_pConv[1]->accessCoefficients(NULL);
    m_pConv[1]->commitCoefficients((vx_int16*)gy);

    m_pImage[0] = pContext->createImage(0, 0, VX_DF_IMAGE_VIRT);  // 0: gaussian
    m_pImage[1] = pContext->createImage(0, 0, VX_DF_IMAGE_VIRT);  // 1: Gx
    m_pImage[2] = pContext->createImage(0, 0, VX_DF_IMAGE_VIRT);  // 2: Gy
    m_pImage[3] = pContext->createImage(0, 0, VX_DF_IMAGE_U8);    // 3: Mag
    m_pImage[4] = pContext->createImage(0, 0, VX_DF_IMAGE_VIRT);  // 4: Phase
    m_pImage[5] = pContext->createImage(0, 0, VX_DF_IMAGE_U8);    // 5: Nonmax

    // TODO bug Fix canny graph to be the actual edge tracing features! (same bug exists in vx_canny.c. inherit fixes from there
    m_pGaussian3x3Node       = new Gaussian3x3Node(pProperties->m_pSubGraph, m_pSrcImage, m_pImage[0]);
    m_pConvolveNode1         = new ConvolveNode(pProperties->m_pSubGraph, m_pImage[0], m_pConv[0], m_pImage[1]);
    m_pConvolveNode2         = new ConvolveNode(pProperties->m_pSubGraph, m_pImage[0], m_pConv[1], m_pImage[2]);
    m_pMagnitudeNode         = new MagnitudeNode(pProperties->m_pSubGraph, m_pImage[1], m_pImage[2], m_pImage[3]);
    m_pPhaseNode             = new PhaseNode(pProperties->m_pSubGraph, m_pImage[1], m_pImage[2], m_pImage[4]);
    m_pNonMaxSuppressionNode = new NonMaxSuppressionNode(pProperties->m_pSubGraph, m_pImage[3], m_pImage[4], m_pImage[5]);
    m_pThresholdNode         = new ThresholdNode(pProperties->m_pSubGraph, m_pImage[5], m_pThreshold, m_pDstImage);

    return (pProperties->m_pSubGraph->verify());
}

// ChannelCombine ------------------------------------------------------------

static vx_status vxChannelCombineKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 5) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image input[4];
    input[0] = (vx_image)ppParamList[0]->handle();
    input[1] = (vx_image)ppParamList[1]->handle();
    input[2] = (vx_image)ppParamList[2]->handle();
    input[3] = (vx_image)ppParamList[3]->handle();
    vx_image output = (vx_image)ppParamList[4]->handle();

    return vxChannelCombine(input, output);
}

ChannelCombineNode::ChannelCombineNode(Graph* pGraph, Image* pSrcImage1, Image* pSrcImage2, Image* pSrcImage3, Image* pSrcImage4, Image* pDstImage) :
                    Node(pGraph)
{
    // store the parameters
    m_apSrcImage[0] = pSrcImage1;
    m_apSrcImage[1] = pSrcImage2;
    m_apSrcImage[2] = pSrcImage3;
    m_apSrcImage[3] = pSrcImage4;
}

vx_status ChannelCombineNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.channelcombine");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxChannelCombineKernelCPP;

    pProperties->m_portCount = 5;
    pProperties->m_apPortList[0]->set(m_apSrcImage[0], pdIn);
    pProperties->m_apPortList[1]->set(m_apSrcImage[1], pdIn);
    pProperties->m_apPortList[2]->set(m_apSrcImage[2], pdIn);
    pProperties->m_apPortList[3]->set(m_apSrcImage[3], pdIn);
    pProperties->m_apPortList[4]->set(m_pDstImage,     pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// ChannelExtract ------------------------------------------------------------

static vx_status vxChannelExtractKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 3) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image  src     = (vx_image )ppParamList[0]->handle();
    vx_scalar channel = (vx_scalar)ppParamList[1]->handle();
    vx_image  dst     = (vx_image )ppParamList[2]->handle();

    return vxChannelExtract(src, channel, dst);
}

ChannelExtractNode::ChannelExtractNode(Graph* pGraph, Image* pSrcImage, Enumerant* pChannel, Image* pDstImage) :
                    Node(pGraph)
{
    // store the parameters
    m_pSrcImage = pSrcImage;
    m_pChannel  = pChannel;
    m_pDstImage = pDstImage;
}

vx_status ChannelExtractNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.ChannelExtract");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxChannelExtractKernelCPP;

    pProperties->m_portCount = 3;
    pProperties->m_apPortList[0]->set(m_pSrcImage, pdIn);
    pProperties->m_apPortList[1]->set(m_pChannel,  pdIn);
    pProperties->m_apPortList[2]->set(m_pDstImage, pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// ConvertColor --------------------------------------------------------------

static vx_status vxConvertColorKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 2) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image src = (vx_image)ppParamList[0]->handle();
    vx_image dst = (vx_image)ppParamList[1]->handle();

    return vxConvertColor(src, dst);
}

ConvertColorNode::ConvertColorNode(Graph* pGraph, Image* pSrcImage, Image* pDstImage) :
                  Node(pGraph)
{
    // store the parameters
    m_pSrcImage = pSrcImage;
    m_pDstImage = pDstImage;
}

vx_status ConvertColorNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.convertcolor");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxConvertColorKernelCPP;

    pProperties->m_portCount = 2;
    pProperties->m_apPortList[0]->set(m_pSrcImage, pdIn);
    pProperties->m_apPortList[1]->set(m_pDstImage, pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// ConvertDepth -----------------------------------------------------------------

static vx_status vxConvertDepthKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 4) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image  input  = (vx_image )ppParamList[0]->handle();
    vx_image  output = (vx_image )ppParamList[1]->handle();
    vx_scalar spol   = (vx_scalar)ppParamList[2]->handle();
    vx_scalar sshf   = (vx_scalar)ppParamList[3]->handle();

    return vxConvertDepth(input, output, spol, sshf);
}

ConvertDepthNode::ConvertDepthNode(Graph* pGraph, Image* pSrcImage, Image* pDstImage, Enumerant* pPolicy, Int32* pShift) :
                  Node(pGraph)
{
    // store the parameters
    m_pSrcImage = pSrcImage;
    m_pDstImage = pDstImage;
    m_pPolicy   = pPolicy;
    m_pShift    = pShift;
}

vx_status ConvertDepthNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.convertdepth");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxConvertDepthKernelCPP;

    pProperties->m_portCount = 4;
    pProperties->m_apPortList[0]->set(m_pSrcImage, pdIn);
    pProperties->m_apPortList[1]->set(m_pDstImage, pdOut);
    pProperties->m_apPortList[2]->set(m_pPolicy,   pdIn);
    pProperties->m_apPortList[3]->set(m_pShift,    pdIn);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// Convolve ------------------------------------------------------------------

static vx_status vxConvolveKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 4) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image       src  = (vx_image      )ppParamList[0]->handle();
    vx_convolution conv = (vx_convolution)ppParamList[1]->handle();
    vx_image       dst  = (vx_image      )ppParamList[2]->handle();
    vx_border_t bordermode = ((BorderMode*)(ppParamList[3]))->m_borderMode;

    return vxConvolve(src, conv, dst, &bordermode);
}

ConvolveNode::ConvolveNode(Graph* pGraph, Image* pSrcImage, Convolution* pConvolution, Image* pDstImage, BorderMode* pBorderMode) :
              Node(pGraph)
{
    // store the parameters
    m_pSrcImage    = pSrcImage;
    m_pConvolution = pConvolution;
    m_pDstImage    = pDstImage;

    if (pBorderMode) {
        m_pBorderMode = pBorderMode;
    }
    else {
        // BUGBUG memory leak here. free this
        m_pBorderMode = new BorderMode(pGraph->context());
    }
}

vx_status ConvolveNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.convolve");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxConvolveKernelCPP;

    pProperties->m_portCount = 4;
    pProperties->m_apPortList[0]->set(m_pSrcImage,    pdIn);
    pProperties->m_apPortList[1]->set(m_pConvolution, pdIn);
    pProperties->m_apPortList[2]->set(m_pDstImage,    pdOut);
    pProperties->m_apPortList[3]->set(m_pBorderMode,  pdIn);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// Fast9Corners --------------------------------------------------------------

/*
static vx_status vxFast9CornersKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 4) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image  src    = (vx_image) ppParamList[0]->handle();
    vx_scalar sens   = (vx_scalar)ppParamList[1]->handle();
    vx_list   points = (vx_list)  ppParamList[2]->handle();
    vx_border_t bordermode = ((BorderMode*)(ppParamList[3]))->m_borderMode;

    return vxFast9Corners(src, sens, points, &bordermode);
}

Fast9CornersNode::Fast9CornersNode(Graph* pGraph, Image* pSrcImage, Float32* pSens, List<vx_keypoint>* pPoints, BorderMode* pBorderMode) :
                  Node(pGraph)
{
    m_pSrcImage = pSrcImage;
    m_pSens     = pSens;
    m_pPoints   = pPoints;

    if (pBorderMode) {
        m_pBorderMode = pBorderMode;
    }
    else {
        // BUGBUG memory leak here. free this
        m_pBorderMode = new BorderMode(pGraph->context());
    }
}

vx_status Fast9CornersNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.fast9corners");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxFast9CornersKernelCPP;

    pProperties->m_portCount = 4;
    pProperties->m_apPortList[0]->set(m_pSrcImage,   pdIn);
    pProperties->m_apPortList[1]->set(m_pSens,       pdIn);
    pProperties->m_apPortList[2]->set(m_pPoints,     pdOut);
    pProperties->m_apPortList[3]->set(m_pBorderMode, pdIn);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}
*/

// Median3x3 -----------------------------------------------------------------

static vx_status vxMedian3x3KernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 3) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image src = (vx_image)ppParamList[0]->handle();
    vx_image dst = (vx_image)ppParamList[1]->handle();
    vx_border_t bordermode = ((BorderMode*)(ppParamList[3]))->m_borderMode;

    return vxMedian3x3(src, dst, &bordermode);
}

Median3x3Node::Median3x3Node(Graph* pGraph, Image* pSrcImage, Image* pDstImage, BorderMode* pBorderMode) :
               Node(pGraph)
{
    // store the parameters
    m_pSrcImage = pSrcImage;
    m_pDstImage = pDstImage;

    if (pBorderMode) {
        m_pBorderMode = pBorderMode;
    }
    else {
        // BUGBUG memory leak here. free this
        m_pBorderMode = new BorderMode(pGraph->context());
    }
}

vx_status Median3x3Node::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.median3x3");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxMedian3x3KernelCPP;

    pProperties->m_portCount = 3;
    pProperties->m_apPortList[0]->set(m_pSrcImage,   pdIn);
    pProperties->m_apPortList[1]->set(m_pDstImage,   pdOut);
    pProperties->m_apPortList[2]->set(m_pBorderMode, pdIn);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// Box3x3 --------------------------------------------------------------------

static vx_status vxBox3x3KernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 3) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image src = (vx_image)ppParamList[0]->handle();
    vx_image dst = (vx_image)ppParamList[1]->handle();
    vx_border_t bordermode = ((BorderMode*)(ppParamList[3]))->m_borderMode;

    return vxBox3x3(src, dst, &bordermode);
}

Box3x3Node::Box3x3Node(Graph* pGraph, Image* pSrcImage, Image* pDstImage, BorderMode* pBorderMode) :
            Node(pGraph)
{
    // store the parameters
    m_pSrcImage = pSrcImage;
    m_pDstImage = pDstImage;

    if (pBorderMode) {
        m_pBorderMode = pBorderMode;
    }
    else {
        // BUGBUG memory leak here. free this
        m_pBorderMode = new BorderMode(pGraph->context());
    }
}

vx_status Box3x3Node::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.box3x3");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxBox3x3KernelCPP;

    pProperties->m_portCount = 3;
    pProperties->m_apPortList[0]->set(m_pSrcImage,   pdIn);
    pProperties->m_apPortList[1]->set(m_pDstImage,   pdOut);
    pProperties->m_apPortList[2]->set(m_pBorderMode, pdIn);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// Gaussian3x3 --------------------------------------------------------------------

static vx_status vxGaussian3x3KernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 3) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image src = (vx_image)ppParamList[0]->handle();
    vx_image dst = (vx_image)ppParamList[1]->handle();
    vx_border_t bordermode = ((BorderMode*)(ppParamList[3]))->m_borderMode;

    return vxGaussian3x3(src, dst, &bordermode);
}

Gaussian3x3Node::Gaussian3x3Node(Graph* pGraph, Image* pSrcImage, Image* pDstImage, BorderMode* pBorderMode) :
                 Node(pGraph)
{
    // store the parameters
    m_pSrcImage = pSrcImage;
    m_pDstImage = pDstImage;

    if (pBorderMode) {
        m_pBorderMode = pBorderMode;
    }
    else {
        // BUGBUG memory leak here. free this
        m_pBorderMode = new BorderMode(pGraph->context());
    }
}

vx_status Gaussian3x3Node::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.gauss3x3");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxGaussian3x3KernelCPP;

    pProperties->m_portCount = 3;
    pProperties->m_apPortList[0]->set(m_pSrcImage,   pdIn);
    pProperties->m_apPortList[1]->set(m_pDstImage,   pdOut);
    pProperties->m_apPortList[2]->set(m_pBorderMode, pdIn);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// Harris --------------------------------------------------------------------

// TODO: implement Harris corners
// In the C version , Harris corners are implemented as an amalgamation of
// other nodes. unclear how to re-use common code between C and C++ here since
// the mechanisms for node and graph assembly are so different. This might just
// be a re-write.

// Histogram -----------------------------------------------------------------

static vx_status vxHistogramKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 2) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image        src  = (vx_image) ppParamList[0]->handle();
    vx_distribution dist = (vx_distribution)ppParamList[1]->handle();

    return vxHistogram(src, dist);
}

HistogramNode::HistogramNode(Graph* pGraph, Image* pSrcImage, Distribution* pDist) :
               Node(pGraph)
{
    // store the parameters
    m_pSrcImage  = pSrcImage;
    m_pDist      = pDist;
}

vx_status HistogramNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.histogram");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxHistogramKernelCPP;

    pProperties->m_portCount = 2;
    pProperties->m_apPortList[0]->set(m_pSrcImage, pdIn);
    pProperties->m_apPortList[1]->set(m_pDist,     pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// EqualizeHist -----------------------------------------------------------------

static vx_status vxEqualizeHistKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 2) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image src = (vx_image)ppParamList[0]->handle();
    vx_image dst = (vx_image)ppParamList[1]->handle();

    return vxEqualizeHist(src, dst);
}

EqualizeHistNode::EqualizeHistNode(Graph* pGraph, Image* pSrcImage, Image* pDstImage) :
                  Node(pGraph)
{
    // store the parameters
    m_pSrcImage = pSrcImage;
    m_pDstImage = pDstImage;
}

vx_status EqualizeHistNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.equalizehist");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxEqualizeHistKernelCPP;

    pProperties->m_portCount = 2;
    pProperties->m_apPortList[0]->set(m_pSrcImage, pdIn);
    pProperties->m_apPortList[1]->set(m_pDstImage, pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// IntegralImage -----------------------------------------------------------------

static vx_status vxIntegralImageKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 2) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image src = (vx_image)ppParamList[0]->handle();
    vx_image dst = (vx_image)ppParamList[1]->handle();

    return vxIntegralImage(src, dst);
}

IntegralImageNode::IntegralImageNode(Graph* pGraph, Image* pSrcImage, Image* pDstImage) :
                   Node(pGraph)
{
    // store the parameters
    m_pSrcImage = pSrcImage;
    m_pDstImage = pDstImage;
}

vx_status IntegralImageNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.integralimage");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxIntegralImageKernelCPP;

    pProperties->m_portCount = 2;
    pProperties->m_apPortList[0]->set(m_pSrcImage, pdIn);
    pProperties->m_apPortList[1]->set(m_pDstImage, pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// TableLookup -----------------------------------------------------------------

static vx_status vxTableLookupKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 3) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image src = (vx_image) ppParamList[0]->handle();
    vx_lut   lut = (vx_lut)ppParamList[1]->handle();
    vx_image dst = (vx_image) ppParamList[2]->handle();

    return vxTableLookup(src, lut, dst);
}

TableLookupNode::TableLookupNode(Graph* pGraph, Image* pSrcImage, GenericLUT* pLUT, Image* pDstImage) :
                 Node(pGraph)
{
    // store the parameters
    m_pSrcImage = pSrcImage;
    m_pLUT      = pLUT;
    m_pDstImage = pDstImage;
}

vx_status TableLookupNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.tablelookup");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxTableLookupKernelCPP;

    pProperties->m_portCount = 3;
    pProperties->m_apPortList[0]->set(m_pSrcImage, pdIn);
    pProperties->m_apPortList[1]->set(m_pLUT,      pdIn);
    pProperties->m_apPortList[2]->set(m_pDstImage, pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// MeanStdDev -----------------------------------------------------------------

static vx_status vxMeanStdDevKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 3) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image  input  = (vx_image) ppParamList[0]->handle();
    vx_scalar mean   = (vx_scalar)ppParamList[1]->handle();
    vx_scalar stddev = (vx_scalar)ppParamList[2]->handle();

    return vxMeanStdDev(input, mean, stddev);
}

MeanStdDevNode::MeanStdDevNode(Graph* pGraph, Image* pSrcImage, Float32* pMean, Float32* pStdDev) :
                Node(pGraph)
{
    // store the parameters
    m_pSrcImage = pSrcImage;
    m_pMean     = pMean;
    m_pStdDev   = pStdDev;
}

vx_status MeanStdDevNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.meanstddev");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxMeanStdDevKernelCPP;

    pProperties->m_portCount = 3;
    pProperties->m_apPortList[0]->set(m_pSrcImage, pdIn);
    pProperties->m_apPortList[1]->set(m_pMean,     pdOut);
    pProperties->m_apPortList[2]->set(m_pStdDev,   pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// MinMaxLoc -----------------------------------------------------------------

/*
static vx_status vxMinMaxLocKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 5) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image       input  = (vx_image)      ppParamList[0]->handle();
    vx_scalar      minVal = (vx_scalar)     ppParamList[1]->handle();
    vx_scalar      maxVal = (vx_scalar)     ppParamList[2]->handle();
    vx_coordinates minLoc = (vx_coordinates)ppParamList[3]->handle();
    vx_coordinates maxLoc = (vx_coordinates)ppParamList[4]->handle();

    return vxMinMaxLoc(input, minVal, maxVal, minLoc, maxLoc);
}

MinMaxLocNode::MinMaxLocNode(Graph* pGraph, Image* pSrcImage,
                             Int64* pMinVal, Int64* pMaxVal,
                             Coordinates* pMinLoc, Coordinates* pMaxLoc) :
               Node(pGraph)
{
    // store the parameters
    m_pSrcImage = pSrcImage;
    m_pMinVal   = pMinVal;
    m_pMaxVal   = pMaxVal;
    m_pMinLoc   = pMinLoc;
    m_pMaxLoc   = pMaxLoc;
}

vx_status MinMaxLocNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.minmaxloc");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxMinMaxLocKernelCPP;

    pProperties->m_portCount = 5;
    pProperties->m_apPortList[0]->set(m_pSrcImage, pdIn);
    pProperties->m_apPortList[1]->set(m_pMinVal,   pdOut);
    pProperties->m_apPortList[2]->set(m_pMaxVal,   pdOut);
    pProperties->m_apPortList[3]->set(m_pMinLoc,   pdOut);
    pProperties->m_apPortList[4]->set(m_pMaxLoc,   pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}
*/

// Magnitude -----------------------------------------------------------------

static vx_status vxMagnitudeKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 3) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image grad_x = (vx_image)ppParamList[0]->handle();
    vx_image grad_y = (vx_image)ppParamList[1]->handle();
    vx_image output = (vx_image)ppParamList[2]->handle();

    return vxMagnitude(grad_x, grad_y, output);
}

MagnitudeNode::MagnitudeNode(Graph* pGraph, Image* pSrcGradX, Image* pSrcGradY, Image* pImageOut) :
               Node(pGraph)
{
    // store the parameters
    m_pSrcGradX = pSrcGradX;
    m_pSrcGradY = pSrcGradY;
    m_pImageOut = pImageOut;
}

vx_status MagnitudeNode::defineNode(NodeProperties *pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.magnitude");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxMagnitudeKernelCPP;

    pProperties->m_portCount = 3;
    pProperties->m_apPortList[0]->set(m_pSrcGradX, pdIn);
    pProperties->m_apPortList[1]->set(m_pSrcGradY, pdIn);
    pProperties->m_apPortList[2]->set(m_pImageOut, pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxuMagnitudeCPP(Image* pSrcGradX, Image* pSrcGradY, Image* pImageOut)
{
    // package up the parameters
    Parameter* apParams[3];
    apParams[0] = pSrcGradX;
    apParams[1] = pSrcGradY;
    apParams[2] = pImageOut;

    return vxMagnitudeKernelCPP(NULL, apParams, 3);
}

// Erode3x3 ------------------------------------------------------------------

static vx_status vxErode3x3KernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 3) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image src = (vx_image)ppParamList[0]->handle();
    vx_image dst = (vx_image)ppParamList[1]->handle();
    vx_border_t bordermode = ((BorderMode*)(ppParamList[2]))->m_borderMode;

    return vxErode3x3(src, dst, &bordermode);
}

Erode3x3Node::Erode3x3Node(Graph* pGraph, Image* pSrcImage, Image* pDstImage, BorderMode* pBorderMode) :
              Node(pGraph)
{
    // store the parameters
    m_pSrcImage = pSrcImage;
    m_pDstImage = pDstImage;

    if (pBorderMode) {
        m_pBorderMode = pBorderMode;
    }
    else {
        // BUGBUG memory leak here. free this
        m_pBorderMode = new BorderMode(pGraph->context());
    }
}

vx_status Erode3x3Node::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.erode3x3");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxErode3x3KernelCPP;

    pProperties->m_portCount = 3;
    pProperties->m_apPortList[0]->set(m_pSrcImage,   pdIn);
    pProperties->m_apPortList[1]->set(m_pDstImage,   pdOut);
    pProperties->m_apPortList[2]->set(m_pBorderMode, pdIn);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// Dilate3x3 -----------------------------------------------------------------

static vx_status vxDilate3x3KernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 3) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image src = (vx_image)ppParamList[0]->handle();
    vx_image dst = (vx_image)ppParamList[1]->handle();
    vx_border_t bordermode = ((BorderMode*)(ppParamList[2]))->m_borderMode;

    return vxDilate3x3(src, dst, &bordermode);
}

Dilate3x3Node::Dilate3x3Node(Graph* pGraph, Image* pSrcImage, Image* pDstImage, BorderMode* pBorderMode) :
               Node(pGraph)
{
    // store the parameters
    m_pSrcImage = pSrcImage;
    m_pDstImage = pDstImage;

    if (pBorderMode) {
        m_pBorderMode = pBorderMode;
    }
    else {
        // BUGBUG memory leak here. free this
        m_pBorderMode = new BorderMode(pGraph->context());
    }
}

vx_status Dilate3x3Node::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.dilate3x3");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxDilate3x3KernelCPP;

    pProperties->m_portCount = 2;
    pProperties->m_apPortList[0]->set(m_pSrcImage,   pdIn);
    pProperties->m_apPortList[1]->set(m_pDstImage,   pdOut);
    pProperties->m_apPortList[2]->set(m_pBorderMode, pdIn);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// Multiply ------------------------------------------------------------------

static vx_status vxMultiplyKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 6) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image    in0    = (vx_image) ppParamList[0]->handle();
    vx_image    in1    = (vx_image) ppParamList[1]->handle();
    vx_scalar   scale  = (vx_scalar)ppParamList[2]->handle();
    vx_scalar   opolicy = (vx_scalar)ppParamList[3]->handle();
    vx_scalar   rpolicy = (vx_scalar)ppParamList[4]->handle();
    vx_image    output = (vx_image) ppParamList[5]->handle();

    return vxMultiply(in0, in1, scale, opolicy, rpolicy, output);
}

MultiplyNode::MultiplyNode(Graph* pGraph, Image* pSrcImage1, Image* pSrcImage2, Float32* pScale, Enumerant* pOPolicy, Enumerant* pRPolicy, Image* pDstImage) :
              Node(pGraph)
{
    // store the parameters
    m_pSrcImage1 = pSrcImage1;
    m_pSrcImage2 = pSrcImage2;
    m_pScale     = pScale;
    m_pOPolicy   = pOPolicy;
    m_pRPolicy   = pRPolicy;
    m_pDstImage  = pDstImage;
}

vx_status MultiplyNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.multiply");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxMultiplyKernelCPP;

    pProperties->m_portCount = 5;
    pProperties->m_apPortList[0]->set(m_pSrcImage1, pdIn);
    pProperties->m_apPortList[1]->set(m_pSrcImage2, pdIn);
    pProperties->m_apPortList[2]->set(m_pScale,     pdIn);
    pProperties->m_apPortList[3]->set(m_pOPolicy,    pdIn);
    pProperties->m_apPortList[4]->set(m_pRPolicy,   pdIn);
    pProperties->m_apPortList[5]->set(m_pDstImage,  pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// OpticalFlowPyrLK ----------------------------------------------------------

// TODO: implement this (note that the C version isn't done either)

static vx_status vxOpticalFlowPyrLKKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
//     if (paramCount != XX) {
//         return VX_ERROR_INVALID_PARAMETERS;
//     }
//
//     // unpack the parameters so we can just use the original c-style kernel code
//     vx_type1 p1 = (vx_type1)ppParamList[0]->handle();
//     vx_type2 p2 = (vx_type2)ppParamList[1]->handle();
//     vx_type3 p3 = (vx_type3)ppParamList[2]->handle();
//     ...

    return vxOpticalFlowPyrLK(/*p1, p2, ...*/);
}

OpticalFlowPyrLKNode::OpticalFlowPyrLKNode(Graph* pGraph /*, p1, p2, ...*/) :
                      Node(pGraph)
{


    // store the parameters
//     m_p1    = p1;
//     m_p2    = p2;
//     m_p3    = p3;
//     ...
}

vx_status OpticalFlowPyrLKNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.optpyrLK");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxOpticalFlowPyrLKKernelCPP;

//     pProperties->m_portCount = XX;
//     pProperties->m_apPortList[0]->set(m_p1, pdIn/Out);
//     pProperties->m_apPortList[1]->set(m_p2, pdIn/Out);
//     pProperties->m_apPortList[2]->set(m_p3, pdIn/Out);
//     ...

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// Phase -----------------------------------------------------------------

static vx_status vxPhaseKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 3) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image grad_x = (vx_image)ppParamList[0]->handle();
    vx_image grad_y = (vx_image)ppParamList[1]->handle();
    vx_image output = (vx_image)ppParamList[2]->handle();

    return vxPhase(grad_x, grad_y, output);
}

PhaseNode::PhaseNode(Graph* pGraph, Image* pSrcGradX, Image* pSrcGradY, Image* pDstImage) :
           Node(pGraph)
{
    // store the parameters
    m_pSrcGradX = pSrcGradX;
    m_pSrcGradY = pSrcGradY;
    m_pDstImage = pDstImage;
}

vx_status PhaseNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.phase");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxPhaseKernelCPP;

    pProperties->m_portCount = 3;
    pProperties->m_apPortList[0]->set(m_pSrcGradX, pdIn);
    pProperties->m_apPortList[1]->set(m_pSrcGradY, pdIn);
    pProperties->m_apPortList[2]->set(m_pDstImage, pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// Pyramid -------------------------------------------------------------------

/*
static vx_status vxPyramidKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    // i think the parameter list is pointless here, because we've already baked them into the sub-graph in "defineNode".
    // admittedly, this is rather heavy-weight in that the graph must be re-verified when params change. lighter weight solution
    // migh be to overload the "process" function with a second version that takes parameters that are mapped a list of exported graph params
    if (pSubGraph == NULL) {
        return VX_ERROR_INVALID_PARAMETERS;
    }
    return pSubGraph->process();
}

PyramidNode::PyramidNode(Graph* pGraph, Image* pSrcImage, Pyramid* pDstGaussian, Pyramid* pDstLaplacian) :
             Node(pGraph)
{
    // store the parameters
    m_pSrcImage     = pSrcImage;
    m_pDstGaussian  = pDstGaussian;
    m_pDstLaplacian = pDstLaplacian;
}

vx_status PyramidNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.Pyramid");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxPyramidKernelCPP;

    pProperties->m_portCount = 3;
    pProperties->m_apPortList[0]->set(m_pSrcImage,     pdIn);
    pProperties->m_apPortList[1]->set(m_pDstGaussian,  pdOut);
    pProperties->m_apPortList[2]->set(m_pDstLaplacian, pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    // generate the sub-graph ---------------------------

    // inherit the context from the graph of which this node is a part
    Context* pContext = graph()->context();

    // create our own new sub-graph
    pProperties->m_pSubGraph = pContext->createGraph();

    if (!pProperties->m_pSubGraph) {
        return VX_ERROR_NO_RESOURCES;
    }

    m_pInterp = pContext->createEnumerant(VX_INTERPOLATION_NEAREST_NEIGHBOR);

    if (m_pDstGaussian)
    {
        m_pCopyImageNodeG = new CopyImageNode(pProperties->m_pSubGraph, m_pSrcImage, m_pDstGaussian->getLevel(0));
        vx_size levels = m_pDstGaussian->levels();
        for (unsigned int level=1; level<levels; level++) {
            Image* pTmp0 = (level-1 == 0) ?
                           m_pSrcImage :
                           m_pDstGaussian->getLevel(level-1);
            Image* pTmp1 = m_pDstGaussian->getLevel(level);
            m_pGImage[level-1] = pContext->createImage(0, 0, VX_DF_IMAGE_VIRT);
            m_pGNode[level-1]  = new Gaussian3x3Node(pProperties->m_pSubGraph, pTmp0, m_pGImage[level-1]);
            m_pGSNode[level-1] = new ScaleImageNode(pProperties->m_pSubGraph, m_pGImage[level-1], pTmp1, m_pInterp);
        }
    }
    if (m_pDstLaplacian)
    {
        // TODO Implement m_pDstLaplacian pyramids correctly (duplicated from broken C version 2014-02-26)
        m_pCopyImageNodeL = new CopyImageNode(pProperties->m_pSubGraph, m_pSrcImage, m_pDstLaplacian->getLevel(0));
        vx_size levels = m_pDstLaplacian->levels();
        for (unsigned int level=1; level<levels; level++) {
            Image* pTmp0 = (level-1 == 0) ?
                            m_pSrcImage :
                            m_pDstLaplacian->getLevel(level-1);
            Image* pTmp1 = m_pDstLaplacian->getLevel(level);
            m_pLImage[level-1] = pContext->createImage(0, 0, VX_DF_IMAGE_VIRT);
            m_pLNode[level-1]  = new Laplacian3x3Node(pProperties->m_pSubGraph, pTmp0, m_pLImage[level-1]);
            m_pLSNode[level-1] = new ScaleImageNode(pProperties->m_pSubGraph, m_pLImage[level-1], pTmp1, m_pInterp);
        }
    }

    return (pProperties->m_pSubGraph->verify());
}
*/

// ScaleImage ----------------------------------------------------------------

/*
static vx_status vxScaleImageKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    vx_status status = VX_SUCCESS;

    if (paramCount != 5) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image    src_image = (vx_image) ppParamList[0]->handle();
    vx_image    dst_image = (vx_image) ppParamList[1]->handle();
    vx_scalar   stype     = (vx_scalar)ppParamList[2]->handle();
    vx_buffer   interm    = (vx_buffer)ppParamList[3]->handle();
    vx_size     size      = (vx_size)  ppParamList[4]->handle();

    void* pLocalData;
    status |= vxAccessBufferRange(interm, 0, size, &pLocalData, VX_READ_AND_WRITE);
    status |= vxScaleImage(src_image, dst_image, stype, (vx_float64*)pLocalData, size);
    status |= vxCommitBufferRange(interm, 0, size, pLocalData);

    return status;
}

ScaleImageNode::ScaleImageNode(Graph* pGraph, Image* pSrcImage, Image* pDstImage, Enumerant* pType) :
                Node(pGraph)
{
    // store the parameters
    m_pSrcImage      = pSrcImage;
    m_pDstImage      = pDstImage;
    m_pType          = pType;
    m_pLocalData     = pGraph->context()->createBuffer(0,0);  // BUGBUG fix these values
    m_pLocalDataSize = pGraph->context()->createSize((vx_size)0ul);
}

vx_status ScaleImageNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.scaleimage");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxScaleImageKernelCPP;

    pProperties->m_portCount = 5;
    pProperties->m_apPortList[0]->set(m_pSrcImage,      pdIn);
    pProperties->m_apPortList[1]->set(m_pDstImage,      pdOut);
    pProperties->m_apPortList[2]->set(m_pType,          pdIn);
    pProperties->m_apPortList[3]->set(m_pLocalData,     pdNonFlowing);
    pProperties->m_apPortList[4]->set(m_pLocalDataSize, pdNonFlowing);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}
*/

// Sobel 3x3 -----------------------------------------------------------------

static vx_status vxSobel3x3KernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 4) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image input  = (vx_image)ppParamList[0]->handle();
    vx_image grad_x = (vx_image)ppParamList[1]->handle();
    vx_image grad_y = (vx_image)ppParamList[2]->handle();
    vx_border_t bordermode = ((BorderMode*)(ppParamList[3]))->m_borderMode;

    return vxSobel3x3(input, grad_x, grad_y, &bordermode);
}

Sobel3x3Node::Sobel3x3Node(Graph* pGraph, Image* pSrcImage, Image* pDstGradX, Image* pDstGradY, BorderMode* pBorderMode) :
              Node(pGraph, pBorderMode)  // BUGBUG: what's the interaction between this border mode and the m_pBorderMode below?? kill one or the other
{
    // store the parameters
    m_pSrcImage = pSrcImage;
    m_pDstGradX = pDstGradX;
    m_pDstGradY = pDstGradY;

    if (pBorderMode) {
        m_pBorderMode = pBorderMode;
    }
    else {
        // BUGBUG memory leak here. free this
        m_pBorderMode = new BorderMode(pGraph->context());
    }
}

vx_status Sobel3x3Node::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.sobel3x3");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxSobel3x3KernelCPP;

    pProperties->m_portCount = 4;
    pProperties->m_apPortList[0]->set(m_pSrcImage,   pdIn);
    pProperties->m_apPortList[1]->set(m_pDstGradX,   pdOut);
    pProperties->m_apPortList[2]->set(m_pDstGradY,   pdOut);
    pProperties->m_apPortList[3]->set(m_pBorderMode, pdIn);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// BUGBUG are we really supporting vxu's for C++? i'm inclined toward "no"...
VX_API_ENTRY vx_status VX_API_CALL vxuSobel3x3CPP(Image* pSrcImage, Image* pDstGradX, Image* pDstGradY, BorderMode* pBorderMode)
{
    // package up the parameters
    Parameter* apParams[4];
    apParams[0] = pSrcImage;
    apParams[1] = pDstGradX;
    apParams[2] = pDstGradY;
    apParams[3] = pBorderMode;

    return vxSobel3x3KernelCPP(NULL, apParams, 4);
}

// Threshold -----------------------------------------------------------------

static vx_status vxThresholdKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 3) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image     src_image = (vx_image)    ppParamList[0]->handle();
    vx_threshold threshold = (vx_threshold)ppParamList[1]->handle();
    vx_image     dst_image = (vx_image)    ppParamList[2]->handle();

    return vxThreshold(src_image, threshold, dst_image);
}

ThresholdNode::ThresholdNode(Graph* pGraph, Image* pSrcImage, Threshold<vx_uint8>* pThreshold, Image* pDstImage) :
               Node(pGraph)
{
    // store the parameters
    m_pSrcImage  = pSrcImage;
    m_pThreshold = pThreshold;
    m_pDstImage  = pDstImage;
}

vx_status ThresholdNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.threshold");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxThresholdKernelCPP;

    pProperties->m_portCount = 3;
    pProperties->m_apPortList[0]->set(m_pSrcImage,  pdIn);
    pProperties->m_apPortList[1]->set(m_pThreshold, pdIn);
    pProperties->m_apPortList[2]->set(m_pDstImage,  pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// WarpPerspective -----------------------------------------------------------

static vx_status vxWarpPerspectiveKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 4) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image  src_image = (vx_image) ppParamList[0]->handle();
    vx_matrix matrix    = (vx_matrix)ppParamList[1]->handle();
    vx_scalar stype     = (vx_scalar)ppParamList[2]->handle();
    vx_image  dst_image = (vx_image) ppParamList[3]->handle();
    vx_border_t bordermode = ((BorderMode*)(ppParamList[4]))->m_borderMode;

    return vxWarpPerspective(src_image, matrix, stype, dst_image, &bordermode);
}

WarpPerspectiveNode::WarpPerspectiveNode(Graph* pGraph, Image* pSrcImage, Matrix<vx_float32>* pMatrix, Enumerant* pInterpType, Image* pDstImage) :
                     Node(pGraph)
{
    // store the parameters
    m_pSrcImage   = pSrcImage;
    m_pMatrix     = pMatrix;
    m_pInterpType = pInterpType;
    m_pDstImage   = pDstImage;
}

vx_status WarpPerspectiveNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.warpperspective");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxWarpPerspectiveKernelCPP;

    pProperties->m_portCount = 5;
    pProperties->m_apPortList[0]->set(m_pSrcImage,   pdIn);
    pProperties->m_apPortList[1]->set(m_pMatrix,     pdIn);
    pProperties->m_apPortList[2]->set(m_pInterpType, pdIn);
    pProperties->m_apPortList[3]->set(m_pDstImage,   pdOut);
    pProperties->m_apPortList[4]->set(m_pBorderMode, pdIn);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// WarpAffine ----------------------------------------------------------------

static vx_status vxWarpAffineKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 4) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image  src_image = (vx_image) ppParamList[0]->handle();
    vx_matrix matrix    = (vx_matrix)ppParamList[1]->handle();
    vx_scalar stype     = (vx_scalar)ppParamList[2]->handle();
    vx_image  dst_image = (vx_image) ppParamList[3]->handle();
    vx_border_t bordermode = ((BorderMode*)(ppParamList[4]))->m_borderMode;

    return vxWarpAffine(src_image, matrix, stype, dst_image, &bordermode);
}

WarpAffineNode::WarpAffineNode(Graph* pGraph, Image* pSrcImage, Matrix<vx_float32>* pMatrix, Enumerant* pInterpType, Image* pDstImage) :
                Node(pGraph)
{
    // store the parameters
    m_pSrcImage   = pSrcImage;
    m_pMatrix     = pMatrix;
    m_pInterpType = pInterpType;
    m_pDstImage   = pDstImage;
}

vx_status WarpAffineNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.warpaffine");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxWarpAffineKernelCPP;

    pProperties->m_portCount = 5;
    pProperties->m_apPortList[0]->set(m_pSrcImage,   pdIn);
    pProperties->m_apPortList[1]->set(m_pMatrix,     pdIn);
    pProperties->m_apPortList[2]->set(m_pInterpType, pdIn);
    pProperties->m_apPortList[3]->set(m_pDstImage,   pdOut);
    pProperties->m_apPortList[4]->set(m_pBorderMode, pdIn);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

//----------------------------------------------------------------------------
// debug nodes
//----------------------------------------------------------------------------

// FReadImage ----------------------------------------------------------------

static vx_status vxFReadImageKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 2) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the String and convert remaining Parameters to vx_types
    // so we can just use the original c-style kernel code
    String* pFilename = (String*)ppParamList[0];
    vx_image img = (vx_image)ppParamList[1]->handle();

    // wrap the filename up as an Array because that's how vxFReadImage wants it, for reasons that escape me
    Array* pArray = pFilename->context()->createArray(VX_TYPE_CHAR, VX_MAX_FILE_NAME);
    pArray->AddArrayItems(VX_MAX_FILE_NAME, pFilename->str());
    vx_array filename = (vx_array)pArray->handle();

    vx_status status = vxFReadImage(filename, img);

    delete pArray;
    return status;
}

FReadImageNode::FReadImageNode(Graph* pGraph, Image* pImage, String* pFilename) :
                Node(pGraph)
{
    // store the parameters
    m_pImage    = pImage;
    m_pFilename = pFilename;
}

vx_status FReadImageNode::defineNode(NodeProperties *pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.freadimage");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxFReadImageKernelCPP;

    // fill in parameter instances
    pProperties->m_portCount = 2;
    pProperties->m_apPortList[0]->set(m_pFilename, pdIn);
    pProperties->m_apPortList[1]->set(m_pImage,    pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// FWriteImage ---------------------------------------------------------------

static vx_status vxFWriteImageKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 2) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the String and convert remaining Parameters to vx_types
    // so we can just use the original c-style kernel code
    vx_image img = (vx_image)ppParamList[0]->handle();
    String* pFilename = (String*)ppParamList[1];

    // wrap the filename up as an Array because that's how vxFReadImage wants it, for reasons that escape me
    Array* pArray = pFilename->context()->createArray(VX_TYPE_CHAR, VX_MAX_FILE_NAME);
    pArray->AddArrayItems(VX_MAX_FILE_NAME, pFilename->str());
    vx_array filename = (vx_array)pArray->handle();

    vx_status status = vxFWriteImage(img, filename);

    delete pArray;
    return status;
}

FWriteImageNode::FWriteImageNode(Graph* pGraph, Image* pImage, String* pFilename) :
                 Node(pGraph)
{
    // store the parameters
    m_pImage    = pImage;
    m_pFilename = pFilename;
}

vx_status FWriteImageNode::defineNode(NodeProperties *pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.fwriteimage");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxFWriteImageKernelCPP;

    pProperties->m_portCount = 2;
    pProperties->m_apPortList[0]->set(m_pImage,    pdIn);
    pProperties->m_apPortList[1]->set(m_pFilename, pdIn);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// CopyImage -----------------------------------------------------------------

static vx_status vxCopyImageKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 2) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image input  = (vx_image)ppParamList[0]->handle();
    vx_image output = (vx_image)ppParamList[1]->handle();

    return vxCopyImage(input, output);
}

CopyImageNode::CopyImageNode(Graph* pGraph, Image* pSrcImage, Image* pDstImage) :
               Node(pGraph)
{
    // store the parameters
    m_pSrcImage = pSrcImage;
    m_pDstImage = pDstImage;
}

vx_status CopyImageNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.copyimage");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxCopyImageKernelCPP;

    pProperties->m_portCount = 2;
    pProperties->m_apPortList[0]->set(m_pSrcImage, pdIn);
    pProperties->m_apPortList[1]->set(m_pDstImage, pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// CopyBuffer -----------------------------------------------------------------

static vx_status vxCopyArrayKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 2) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_array src = (vx_array)ppParamList[0]->handle();
    vx_array dst = (vx_array)ppParamList[1]->handle();

    return vxCopyArray(src, dst);
}

CopyArrayNode::CopyArrayNode(Graph* pGraph, Array* pSrcArray, Array* pDstArray) :
                Node(pGraph)
{
    // store the parameters
    m_pSrcArray = pSrcArray;
    m_pDstArray = pDstArray;
}

vx_status CopyArrayNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.copyarray");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxCopyArrayKernelCPP;

    pProperties->m_portCount = 2;
    pProperties->m_apPortList[0]->set(m_pSrcArray, pdIn);
    pProperties->m_apPortList[1]->set(m_pDstArray, pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

//----------------------------------------------------------------------------
// "extras" nodes
//----------------------------------------------------------------------------

// EuclideanNonMaxSuppressionHarris ------------------------------------------------

static vx_status vxEuclideanNonMaxSuppressionHarrisKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 4) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image  src = (vx_image )ppParamList[0]->handle();
    vx_scalar thr = (vx_scalar)ppParamList[1]->handle();
    vx_scalar rad = (vx_scalar)ppParamList[2]->handle();
    vx_image  dst = (vx_image )ppParamList[3]->handle();

    return vxEuclideanNonMaxSuppressionHarris(src, thr, rad, dst);
}

EuclideanNonMaxSuppressionHarrisNode::EuclideanNonMaxSuppressionHarrisNode(Graph* pGraph,
                                                               Image* pSrcImage, Float32* pThreshold, Float32* pRadius, Image* pDstImage) :
                                Node(pGraph)
{
    // store the parameters
    m_pSrcImage  = pSrcImage;
    m_pThreshold = pThreshold;
    m_pRadius    = pRadius;
    m_pDstImage  = pDstImage;
}

vx_status EuclideanNonMaxSuppressionHarrisNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.euclideannonmaxsuppression");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxEuclideanNonMaxSuppressionHarrisKernelCPP;

    pProperties->m_portCount = 4;
    pProperties->m_apPortList[0]->set(m_pSrcImage,  pdIn);
    pProperties->m_apPortList[1]->set(m_pThreshold, pdIn);
    pProperties->m_apPortList[2]->set(m_pRadius,    pdIn);
    pProperties->m_apPortList[3]->set(m_pDstImage,  pdOut);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// NonMaxSuppression -----------------------------------------------------------------

static vx_status vxNonMaxSuppressionKernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 3) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image i_mag  = (vx_image)ppParamList[0]->handle();
    vx_image i_ang  = (vx_image)ppParamList[1]->handle();
    vx_image i_edge = (vx_image)ppParamList[2]->handle();
    vx_border_t bordermode = ((BorderMode*)(ppParamList[3]))->m_borderMode;

    return vxNonMaxSuppression(i_mag, i_ang, i_edge, &bordermode);
}

NonMaxSuppressionNode::NonMaxSuppressionNode(Graph* pGraph, Image* pSrcImageMag, Image* pSrcImageAng, Image* pDstImageEdge) :
         Node(pGraph)
{
    // store the parameters
    m_pSrcImageMag  = pSrcImageMag;
    m_pSrcImageAng  = pSrcImageAng;
    m_pDstImageEdge = pDstImageEdge;
}

vx_status NonMaxSuppressionNode::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.nonmaxsuppression");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxNonMaxSuppressionKernelCPP;

    pProperties->m_portCount = 3;
    pProperties->m_apPortList[0]->set(m_pSrcImageMag,  pdIn);
    pProperties->m_apPortList[1]->set(m_pSrcImageAng,  pdIn);
    pProperties->m_apPortList[2]->set(m_pDstImageEdge, pdOut);
    pProperties->m_apPortList[3]->set(m_pBorderMode,   pdIn);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}

// Laplacian3x3 -----------------------------------------------------------------

static vx_status vxLaplacian3x3KernelCPP(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount)
{
    if (paramCount != 3) {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    // unpack the parameters so we can just use the original c-style kernel code
    vx_image src = (vx_image)ppParamList[0]->handle();
    vx_image dst = (vx_image)ppParamList[1]->handle();
    vx_border_t bordermode = ((BorderMode*)(ppParamList[3]))->m_borderMode;

    return vxLaplacian3x3(src, dst, &bordermode);
}

Laplacian3x3Node::Laplacian3x3Node(Graph* pGraph, Image* pSrcImage, Image* pDstImage, BorderMode* pBorderMode) :
                  Node(pGraph)
{
    // store the parameters
    m_pSrcImage = pSrcImage;
    m_pDstImage = pDstImage;

    if (pBorderMode) {
        m_pBorderMode = pBorderMode;
    }
    else {
        // BUGBUG memory leak here. free this
        m_pBorderMode = new BorderMode(pGraph->context());
    }
}

vx_status Laplacian3x3Node::defineNode(NodeProperties* pProperties)
{
    strcpy(pProperties->m_nodeName, "org.khronos.openvx.laplacian3x3");

    pProperties->m_kernels[0].target = ttCPU;
    pProperties->m_kernels[0].kernel = vxLaplacian3x3KernelCPP;

    pProperties->m_portCount = 3;
    pProperties->m_apPortList[0]->set(m_pSrcImage,   pdIn);
    pProperties->m_apPortList[1]->set(m_pDstImage,   pdOut);
    pProperties->m_apPortList[2]->set(m_pBorderMode, pdNonFlowing);

    pProperties->m_localDataSize = 0;  // BUGBUG fix these values

    vx_neighborhood_size_t neighborhoodSize = { 0,0,0,0 };
    vx_tile_block_size_t   tileBlockSize    = { 0,0 };

    pProperties->m_neighborhoodSize = neighborhoodSize;
    pProperties->m_tileBlockSize    = tileBlockSize;

    return VX_SUCCESS;
}
