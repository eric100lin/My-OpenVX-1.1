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

#ifndef _VX_NODE_HPP_
#define _VX_NODE_HPP_

#include <VX/vx_lib_debug.h>

#define NODE_MAX_PARAMS 8

namespace OpenVX {

    #define VX_MAX_NODE_NAME 64
    #define VX_MAX_KERNEL_COUNT 8

    typedef enum TargetType_ {
        ttCPU,
        ttCUDA
    } TargetType;

    typedef vx_status (*KernelFunction)(Graph* pSubGraph, Parameter** ppParamList, vx_uint32 paramCount);

    class VX_CLASS Kernel {
    public:
        TargetType     target;   // where does this kernel run? e.g. CPU/GPU/etc.
        KernelFunction kernel;
    };

    // immutable and static properties of a derived node
    class VX_CLASS NodeProperties {
    public:
        // immutable (hard-wired in code)
        vx_char                 m_nodeName[VX_MAX_NODE_NAME];
        Kernel                  m_kernels[VX_MAX_KERNEL_COUNT];
        vx_size                 m_localDataSize;   // should this even be here or should it be a class member as in ScaleImageNode?
        vx_neighborhood_size_t  m_neighborhoodSize;
        vx_tile_block_size_t    m_tileBlockSize;
        Graph*                  m_pSubGraph;    // non-NULL only if node is implemented as a graph

        // static (derived from arguments to constructor)
        vx_size                 m_portCount;
        Port*                   m_apPortList[NODE_MAX_PARAMS];

    };

    // abstract base class from which all nodes are derived

    class VX_CLASS Node {

        friend class Context;
        friend class Graph;

        Parameter*   m_apParams[NODE_MAX_PARAMS];  // array of raw Parameters, to be passed into the kernel

        Parameter*   m_apParamsIn[NODE_MAX_PARAMS];
        unsigned int m_paramsInCount;
        Parameter*   m_apParamsOut[NODE_MAX_PARAMS];
        unsigned int m_paramsOutCount;

        Graph*       m_pGraph;

        bool m_dirty;       // node needs re-initialization because it never has been init'd or because params have changed
        bool m_scheduled;   // for use by the graph manager

        bool init();                                // used by the graph manager to set up nodes at verification time
        vx_status execute(TargetType targetType);   // used by the graph manager to trigger execution of the node on the given target

    protected:
        // immutable and static properties of the derived node
        NodeProperties m_properties;

        // static properties of base node. configurable per node instance, but static once set
        // (correspond to arguments to the constructor)
        BorderMode* m_pBorderMode;

        // dynamic properties. can be configured on the fly, at runtime
        // (correspond to setter functions in the class)
        void* m_pLocalData;

        // system-generated properties
        vx_status m_status;
        vx_perf_t m_perf;

        // BUGBUG do we need any of these?
        // vx_nodecomplete_f callback;
        // vx_size   tileMemorySize;       settable by graph maanger
        // void*     tileMemory;            ditto

        Node(Graph* pGraph, BorderMode* pBorderMode = NULL);

        // pure virtual functions that must be defined by derived nodes
        // defineNode must fully flesh out the generalized values in NodeProperties for consumption/use by graph manager, etc.
        virtual vx_status defineNode(NodeProperties *pProperties) = 0;

        virtual vx_status initialize() = 0;     // called at graph validation??
        virtual vx_status deinitialize() = 0;   // called at graph invalidation??

        void setDirty();                        // should be called by derived node when "re-definition" is needed because a parameter has changed.
                                                // will provoke another (lazy) call to defineNode()

    public:
        virtual ~Node();          // vxReleaseNode

        // setters for dynamic properties
        vx_status setLocalData(void* pData);

        // getters
        vx_char*        name();
        vx_status       status();
        vx_perf_t*      perf();

        Graph*          graph()   { return m_pGraph; }

        // BUGBUG how many of these do we need?
        // vx_uint32               numParameters();  // needed? why would they need to know?
        // vx_size                 localDataSize();
        // void*                   localData();
        // vx_neighborhood_size_t* neighborhoodSize();
        // vx_tile_block_size_t*   tileBlockSize();
        // vx_border_t*            borderMode();
    };


    // pre-declarations, as needed
    class ConvolveNode;
    class Gaussian3x3Node;
    class Laplacian3x3Node;
    class MagnitudeNode;
    class PhaseNode;
    class ScaleImageNode;
    class ThresholdNode;
    class NonMaxSuppressionNode;
    class CopyImageNode;


    //---------------------------------------------------------------------------
    // derived nodes
    //---------------------------------------------------------------------------

    class VX_CLASS AbsDiffNode : public Node {

        Image* m_pSrcImage1;
        Image* m_pSrcImage2;
        Image* m_pDstImage;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        AbsDiffNode(Graph* pGraph,
                    Image* pSrcImage1, Image* pSrcImage2, Image* pDstImage);
        ~AbsDiffNode() {}

        // setters
        void setSrcImage1(Image* pImage)  { m_pSrcImage1 = pImage; setDirty(); }
        void setSrcImage2(Image* pImage)  { m_pSrcImage2 = pImage; setDirty(); }
        void setDstImage (Image* pImage)  { m_pDstImage  = pImage; setDirty(); }
    };

    class AccumulateNode : public Node {

        Image* m_pSrcImage;
        Image* m_pAccum;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        AccumulateNode(Graph* pGraph,
                       Image* pSrcImage, Image* pAccum);
        ~AccumulateNode() {}

        // setters
        void setSrcImage  (Image* pImage)  { m_pSrcImage = pImage; setDirty(); }
        void setAccumImage(Image* pImage)  { m_pAccum    = pImage; setDirty(); }
    };


    class VX_CLASS AccumulateWeightedNode : public Node {

        Image*         m_pSrcImage;
        Float32*       m_pWeight;
        Image*         m_pAccum;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        AccumulateWeightedNode(Graph* pGraph,
                               Image* pSrcImage, Float32* pWeight, Image* pAccum);
        ~AccumulateWeightedNode() {}

        // setters
        void setSrcImage  (Image* pImage)          { m_pSrcImage = pImage;  setDirty(); }
        void setWeight    (Float32* pWeight) { m_pWeight   = pWeight; setDirty(); }
        void setAccumImage(Image* pImage)          { m_pAccum    = pImage;  setDirty(); }
    };


    class VX_CLASS AccumulateSquareNode : public Node {

        Image*         m_pSrcImage;
        Float32* m_pWeight;
        Image*         m_pAccum;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        AccumulateSquareNode(Graph* pGraph,
                             Image* pSrcImage, Float32* pWeight, Image* pAccum);
        ~AccumulateSquareNode() {}

        // setters
        void setSrcImage  (Image* pImage)          { m_pSrcImage = pImage;  setDirty(); }
        void setWeight    (Float32* pWeight) { m_pWeight   = pWeight; setDirty(); }
        void setAccumImage(Image* pImage)          { m_pAccum    = pImage;  setDirty(); }
    };


    class VX_CLASS AdditionNode : public Node {

        Image*           m_pSrcImage1;
        Image*           m_pSrcImage2;
        Enumerant* m_pPolicy;
        Image*           m_pDstImage;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        AdditionNode(Graph* pGraph,
                     Image* pSrcImage1, Image* pSrcImage2, Enumerant* pPolicy, Image* pDstImage);
        ~AdditionNode() {}

        // setters
        void setSrcImage1(Image* pImage)            { m_pSrcImage1 = pImage;  setDirty(); }
        void setSrcImage2(Image* pImage)            { m_pSrcImage2 = pImage;  setDirty(); }
        void setPolicy   (Enumerant* pPolicy) { m_pPolicy    = pPolicy; setDirty(); }
        void setDstImage (Image* pImage)            { m_pDstImage  = pImage;  setDirty(); }
    };


    class VX_CLASS SubtractionNode : public Node {

        Image*           m_pSrcImage1;
        Image*           m_pSrcImage2;
        Enumerant* m_pPolicy;
        Image*           m_pDstImage;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        SubtractionNode(Graph* pGraph,
                        Image* pSrcImage1, Image* pSrcImage2, Enumerant* pPolicy, Image* pDstImage);
        ~SubtractionNode() {}

        // setters
        void setSrcImage1(Image* pImage)            { m_pSrcImage1 = pImage;  setDirty(); }
        void setSrcImage2(Image* pImage)            { m_pSrcImage2 = pImage;  setDirty(); }
        void setPolicy   (Enumerant* pPolicy) { m_pPolicy    = pPolicy; setDirty(); }
        void setDstImage (Image* pImage)            { m_pDstImage  = pImage;  setDirty(); }
    };


    class VX_CLASS AndNode : public Node {

        Image* m_pSrcImage1;
        Image* m_pSrcImage2;
        Image* m_pDstImage;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        AndNode(Graph* pGraph,
                Image* pSrcImage1, Image* pSrcImage2, Image* pDstImage);
        ~AndNode() {}

        // setters
        void setSrcImage1(Image* pImage) { m_pSrcImage1 = pImage; setDirty(); }
        void setSrcImage2(Image* pImage) { m_pSrcImage2 = pImage; setDirty(); }
        void setDstImage (Image* pImage) { m_pDstImage  = pImage; setDirty(); }
    };


    class VX_CLASS OrNode : public Node {

        Image* m_pSrcImage1;
        Image* m_pSrcImage2;
        Image* m_pDstImage;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        OrNode(Graph* pGraph,
               Image* pSrcImage1, Image* pSrcImage2, Image* pDstImage);
        ~OrNode() {}

        // setters
        void setSrcImage1(Image* pImage) { m_pSrcImage1 = pImage; setDirty(); }
        void setSrcImage2(Image* pImage) { m_pSrcImage2 = pImage; setDirty(); }
        void setDstImage (Image* pImage) { m_pDstImage  = pImage; setDirty(); }
    };


    class VX_CLASS XorNode : public Node {

        Image* m_pSrcImage1;
        Image* m_pSrcImage2;
        Image* m_pDstImage;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        XorNode(Graph* pGraph,
                Image* pSrcImage1, Image* pSrcImage2, Image* pDstImage);
        ~XorNode() {}

        // setters
        void setSrcImage1(Image* pImage) { m_pSrcImage1 = pImage; setDirty(); }
        void setSrcImage2(Image* pImage) { m_pSrcImage2 = pImage; setDirty(); }
        void setDstImage (Image* pImage) { m_pDstImage  = pImage; setDirty(); }
    };


    class VX_CLASS NotNode : public Node {

        Image* m_pSrcImage;
        Image* m_pDstImage;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        NotNode(Graph* pGraph,
                Image* pSrcImage, Image* pDstImage);
        ~NotNode() {}

        // setters
        void setSrcImage(Image* pImage) { m_pSrcImage = pImage; setDirty(); }
        void setDstImage(Image* pImage) { m_pDstImage = pImage; setDirty(); }
    };


    class VX_CLASS CannyEdgeDetectorNode : public Node {

        Image*                 m_pSrcImage;
        Image*                 m_pDstImage;
        Threshold<vx_uint8>*   m_pThreshold;

        // components of the subgraph
        Convolution*           m_pConv[2];
        Image*                 m_pImage[6];
        Gaussian3x3Node*       m_pGaussian3x3Node;
        ConvolveNode*          m_pConvolveNode1;
        ConvolveNode*          m_pConvolveNode2;
        MagnitudeNode*         m_pMagnitudeNode;
        PhaseNode*             m_pPhaseNode;
        NonMaxSuppressionNode* m_pNonMaxSuppressionNode;
        ThresholdNode*         m_pThresholdNode;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        CannyEdgeDetectorNode(Graph* pGraph,
                              Image* pSrcImage, Image* pDstImage, Threshold<vx_uint8>* pThreshold);
        ~CannyEdgeDetectorNode() {}

        // setters
        void setSrcImage (Image* pImage)                { m_pSrcImage  = pImage;  setDirty(); }
        void setDstImage (Image* pImage)                { m_pDstImage  = pImage;  setDirty(); }
        void setThreshold(Threshold<vx_uint8>* pThresh) { m_pThreshold = pThresh; setDirty(); }
    };


    class VX_CLASS ChannelCombineNode : public Node {

        Image* m_apSrcImage[4];
        Image* m_pDstImage;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        ChannelCombineNode(Graph* pGraph,
                           Image* pSrcImage1, Image* pSrcImage2, Image* pSrcImage3, Image* pSrcImage4, Image* pDstImage);
        ~ChannelCombineNode() {}

        // setters
        void setSrcImage1(Image* pImage) { m_apSrcImage[0] = pImage; setDirty(); }
        void setSrcImage2(Image* pImage) { m_apSrcImage[1] = pImage; setDirty(); }
        void setSrcImage3(Image* pImage) { m_apSrcImage[2] = pImage; setDirty(); }
        void setSrcImage4(Image* pImage) { m_apSrcImage[3] = pImage; setDirty(); }
        void setDstImage (Image* pImage) { m_pDstImage     = pImage; setDirty(); }
    };


    class VX_CLASS ChannelExtractNode : public Node {

        Image*           m_pSrcImage;
        Enumerant* m_pChannel;
        Image*           m_pDstImage;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        ChannelExtractNode(Graph* pGraph,
                           Image* pSrcImage, Enumerant* pChannel, Image* pDstImage);
        ~ChannelExtractNode() {}

        // setters
        void setSrcImage(Image* pImage)             { m_pSrcImage = pImage;   setDirty(); }
        void setChannel (Enumerant* pChannel) { m_pChannel  = pChannel; setDirty(); }
        void setDstImage(Image* pImage)             { m_pDstImage = pImage;   setDirty(); }
    };


    class VX_CLASS ConvertColorNode : public Node {

        Image* m_pSrcImage;
        Image* m_pDstImage;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        ConvertColorNode(Graph* pGraph,
                         Image* pSrcImage, Image* pDstImage);
        ~ConvertColorNode() {}

        // setters
        void setSrcImage(Image* pImage) { m_pSrcImage = pImage; setDirty(); }
        void setDstImage(Image* pImage) { m_pDstImage = pImage; setDirty(); }
    };


    class VX_CLASS ConvertDepthNode : public Node {

        Image*            m_pSrcImage;
        Image*            m_pDstImage;
        Enumerant*  m_pPolicy;
        Int32* m_pShift;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        ConvertDepthNode(Graph* pGraph,
                         Image* pSrcImage, Image* pDstImage, Enumerant* pPolicy, Int32* pShift);
        ~ConvertDepthNode() {}

        // setters
        void setSrcImage(Image* pImage)            { m_pSrcImage = pImage;  setDirty(); }
        void setDstImage(Image* pImage)            { m_pDstImage = pImage;  setDirty(); }
        void setPolicy  (Enumerant* pPolicy) { m_pPolicy   = pPolicy; setDirty(); }
        void setShift   (Int32* pShift) { m_pShift    = pShift;  setDirty(); }
    };


    class VX_CLASS ConvolveNode : public Node {

        Image*       m_pSrcImage;
        Convolution* m_pConvolution;
        Image*       m_pDstImage;
        BorderMode*  m_pBorderMode;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        ConvolveNode(Graph* pGraph,
                     Image* pSrcImage, Convolution* pConvolution, Image* pDstImage,
                     BorderMode* pBorderMode = NULL);
        ~ConvolveNode() {}

        // setters
        void setSrcImage   (Image* pImage)             { m_pSrcImage    = pImage;       setDirty(); }
        void setDstImage   (Image* pImage)             { m_pDstImage    = pImage;       setDirty(); }
        void setConvolution(Convolution* pConvolution) { m_pConvolution = pConvolution; setDirty(); }
        void setBorderMode (BorderMode* pBorderMode)   { m_pBorderMode  = pBorderMode;  setDirty(); }
    };

/*
    class VX_CLASS Fast9CornersNode : public Node {

        Image*              m_pSrcImage;
        Float32* m_pSens;
        List<vx_keypoint>*  m_pPoints;
        BorderMode*         m_pBorderMode;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        Fast9CornersNode(Graph* pGraph,
                         Image* pSrcImage, Float32* pSens, List<vx_keypoint>* pPoints,
                         BorderMode* pBorderMode = NULL);
        ~Fast9CornersNode() {}

        // setters
        void setSrcImage   (Image* pImage)              { m_pSrcImage   = pImage;      setDirty(); }
        void setSensitivity(Float32* pSens)  { m_pSens       = pSens;       setDirty(); }
        void setPoints     (List<vx_keypoint>* pPoints) { m_pPoints     = pPoints;     setDirty(); }
        void setBorderMode (BorderMode* pBorderMode)    { m_pBorderMode = pBorderMode; setDirty(); }
    };
*/


    class VX_CLASS Median3x3Node : public Node {

        Image*      m_pSrcImage;
        Image*      m_pDstImage;
        BorderMode* m_pBorderMode;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        Median3x3Node(Graph* pGraph,
                      Image* pSrcImage, Image* pDstImage,
                      BorderMode* pBorderMode = NULL);
        ~Median3x3Node() {}

        // setters
        void setSrcImage  (Image* pImage)           { m_pSrcImage   = pImage;      setDirty(); }
        void setDstImage  (Image* pImage)           { m_pDstImage   = pImage;      setDirty(); }
        void setBorderMode(BorderMode* pBorderMode) { m_pBorderMode = pBorderMode; setDirty(); }
    };


    class VX_CLASS Box3x3Node : public Node {

        Image*      m_pSrcImage;
        Image*      m_pDstImage;
        BorderMode* m_pBorderMode;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        Box3x3Node(Graph* pGraph,
                   Image* pSrcImage, Image* pDstImage,
                   BorderMode* pBorderMode = NULL);
        ~Box3x3Node() {}

        // setters
        void setSrcImage  (Image* pImage)           { m_pSrcImage   = pImage;      setDirty(); }
        void setDstImage  (Image* pImage)           { m_pDstImage   = pImage;      setDirty(); }
        void setBorderMode(BorderMode* pBorderMode) { m_pBorderMode = pBorderMode; setDirty(); }
    };


    class VX_CLASS Gaussian3x3Node : public Node {

        Image*      m_pSrcImage;
        Image*      m_pDstImage;
        BorderMode* m_pBorderMode;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        Gaussian3x3Node(Graph* pGraph,
                        Image* pSrcImage, Image* pDstImage,
                        BorderMode* pBorderMode = NULL);
        ~Gaussian3x3Node() {}

        // setters
        void setSrcImage  (Image* pImage)           { m_pSrcImage   = pImage;      setDirty(); }
        void setDstImage  (Image* pImage)           { m_pDstImage   = pImage;      setDirty(); }
        void setBorderMode(BorderMode* pBorderMode) { m_pBorderMode = pBorderMode; setDirty(); }
    };


    class VX_CLASS HistogramNode : public Node {

        Image*        m_pSrcImage;
        Distribution* m_pDist;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        HistogramNode(Graph* pGraph,
                      Image* pSrcImage, Distribution* pDist);
        ~HistogramNode() {}

        // setters
        void setSrcImage (Image* pImage)       { m_pSrcImage = pImage; setDirty(); }
        void setHistogram(Distribution* pDist) { m_pDist     = pDist;  setDirty(); }
    };


    class VX_CLASS EqualizeHistNode : public Node {

        Image* m_pSrcImage;
        Image* m_pDstImage;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        EqualizeHistNode(Graph* pGraph,
                         Image* pSrcImage, Image* pDstImage);
        ~EqualizeHistNode() {}

        // setters
        void setSrcImage(Image* pImage) { m_pSrcImage = pImage; setDirty(); }
        void setDstImage(Image* pImage) { m_pDstImage = pImage; setDirty(); }
    };


    class VX_CLASS IntegralImageNode : public Node {

        Image* m_pSrcImage;
        Image* m_pDstImage;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        IntegralImageNode(Graph* pGraph,
                          Image *pSrcImage, Image* pDstImage);
        ~IntegralImageNode() {}

        // setters
        void setSrcImage(Image* pImage) { m_pSrcImage = pImage; setDirty(); }
        void setDstImage(Image* pImage) { m_pDstImage = pImage; setDirty(); }
    };


    class VX_CLASS TableLookupNode : public Node {

        Image* m_pSrcImage;
        GenericLUT*  m_pLUT;
        Image* m_pDstImage;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        TableLookupNode(Graph* pGraph,
                        Image* pSrcImage, GenericLUT* pLUT, Image* pDstImage);
        ~TableLookupNode() {}

        // setters
        void setSrcImage(Image* pImage)    { m_pSrcImage = pImage; setDirty(); }
        void setLUT     (GenericLUT* pLUT) { m_pLUT      = pLUT;   setDirty(); }
        void setDstImage(Image* pImage)    { m_pDstImage = pImage; setDirty(); }
    };


    class VX_CLASS MeanStdDevNode : public Node {

        Image*              m_pSrcImage;
        Float32* m_pMean;
        Float32* m_pStdDev;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        MeanStdDevNode(Graph* pGraph,
                       Image* pSrcImage, Float32* pMean, Float32* pStdDev);
        ~MeanStdDevNode() {}

        // setters
        void setSrcImage(Image* pImage)               { m_pSrcImage = pImage;  setDirty(); }
        void setMean    (Float32* pMean)   { m_pMean     = pMean;   setDirty(); }
        void setStdDev  (Float32* pStdDev) { m_pStdDev   = pStdDev; setDirty(); }
    };

/*
    class VX_CLASS MinMaxLocNode : public Node {

        Image*            m_pSrcImage;
        Int64* m_pMinVal;
        Int64* m_pMaxVal;
        Coordinates*      m_pMinLoc;
        Coordinates*      m_pMaxLoc;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        MinMaxLocNode(Graph* pGraph,
                      Image* pSrcImage,
                      Int64* pMinVal, Int64* pMaxVal,
                      Coordinates* pMinLoc, Coordinates* pMaxLoc);
        ~MinMaxLocNode() {}

        // setters
        void setSrcImage(Image* pImage)             { m_pSrcImage = pImage;  setDirty(); }
        void setMinVal  (Int64* pMinVal) { m_pMinVal   = pMinVal; setDirty(); }
        void setMaxVal  (Int64* pMaxVal) { m_pMaxVal   = pMaxVal; setDirty(); }
        void setMinLoc  (Coordinates* pMinLoc)      { m_pMinLoc   = pMinLoc; setDirty(); }
        void setMaxLoc  (Coordinates* pMaxLoc)      { m_pMaxLoc   = pMaxLoc; setDirty(); }
    };
*/

    class VX_CLASS MagnitudeNode : public Node {

        Image* m_pSrcGradX;
        Image* m_pSrcGradY;
        Image* m_pImageOut;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        MagnitudeNode(Graph* pGraph,
                      Image* pSrcGradX, Image* pSrcGradY, Image* pImageOut);
        ~MagnitudeNode() {}

        // setters
        void setSrcGradX(Image* pImage) { m_pSrcGradX = pImage; setDirty(); }
        void setSrcGradY(Image* pImage) { m_pSrcGradY = pImage; setDirty(); }
        void setDstImage(Image* pImage) { m_pImageOut = pImage; setDirty(); }
    };


    class VX_CLASS Erode3x3Node : public Node {

        Image*      m_pSrcImage;
        Image*      m_pDstImage;
        BorderMode* m_pBorderMode;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        Erode3x3Node(Graph* pGraph,
                     Image* pSrcImage, Image* pDstImage,
                     BorderMode* pBorderMode = NULL);
        ~Erode3x3Node() {}

        // setters
        void setSrcImage  (Image* pImage)           { m_pSrcImage   = pImage;      setDirty(); }
        void setDstImage  (Image* pImage)           { m_pDstImage   = pImage;      setDirty(); }
        void setBorderMode(BorderMode* pBorderMode) { m_pBorderMode = pBorderMode; setDirty(); }
    };


    class VX_CLASS Dilate3x3Node : public Node {

        Image*      m_pSrcImage;
        Image*      m_pDstImage;
        BorderMode* m_pBorderMode;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        Dilate3x3Node(Graph* pGraph,
                      Image* pSrcImage, Image* pDstImage,
                      BorderMode* pBorderMode = NULL);
        ~Dilate3x3Node() {}

        // setters
        void setSrcImage  (Image* pImage)           { m_pSrcImage   = pImage;      setDirty(); }
        void setDstImage  (Image* pImage)           { m_pDstImage   = pImage;      setDirty(); }
        void setBorderMode(BorderMode* pBorderMode) { m_pBorderMode = pBorderMode; setDirty(); }
    };


    class VX_CLASS MultiplyNode : public Node {

        Image*              m_pSrcImage1;
        Image*              m_pSrcImage2;
        Float32*            m_pScale;
        Enumerant*          m_pOPolicy;
        Enumerant*          m_pRPolicy;
        Image*              m_pDstImage;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        MultiplyNode(Graph* pGraph,
                     Image* pSrcImage1, Image* pSrcImage2, Float32* pScale, Enumerant* pOPolicy, Enumerant* pRPolicy, Image* pDstImage);
        ~MultiplyNode() {}

        // setters
        void setSrcImage1(Image* pImage)              { m_pSrcImage1 = pImage;  setDirty(); }
        void setSrcImage2(Image* pImage)              { m_pSrcImage2 = pImage;  setDirty(); }
        void setDstImage (Image* pImage)              { m_pDstImage  = pImage;  setDirty(); }
        void setScale    (Float32* pScale) { m_pScale     = pScale;  setDirty(); }
        void setPolicy   (Enumerant* pPolicy)   { m_pOPolicy    = pPolicy; setDirty(); }
    };


    class VX_CLASS OpticalFlowPyrLKNode : public Node {

        //Image*      m_p1;
        //...

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        OpticalFlowPyrLKNode(Graph* pGraph
                     /*p1, p2, ...*/);
        ~OpticalFlowPyrLKNode() {}
    };


    class VX_CLASS PhaseNode : public Node {

        Image* m_pSrcGradX;
        Image* m_pSrcGradY;
        Image* m_pDstImage;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        PhaseNode(Graph* pGraph,
                  Image* pSrcGradX, Image* pSrcGradY, Image* pDstImage);
        ~PhaseNode() {}

        // setters
        void setSrcGradX(Image* pImage) { m_pSrcGradX = pImage; setDirty(); }
        void setSrcGradY(Image* pImage) { m_pSrcGradY = pImage; setDirty(); }
        void setDstImage(Image* pImage) { m_pDstImage = pImage; setDirty(); }
    };

/*
    class VX_CLASS PyramidNode : public Node {

        Image*   m_pSrcImage;
        Pyramid* m_pDstGaussian;
        Pyramid* m_pDstLaplacian;

        // components of the subgraph
        #define PYRAMID_MAX_LEVELS 16

        Enumerant*  m_pInterp;

        CopyImageNode*    m_pCopyImageNodeG;
        Image*            m_pGImage[PYRAMID_MAX_LEVELS];
        Gaussian3x3Node*  m_pGNode[PYRAMID_MAX_LEVELS];
        ScaleImageNode*   m_pGSNode[PYRAMID_MAX_LEVELS];

        CopyImageNode*    m_pCopyImageNodeL;
        Image*            m_pLImage[PYRAMID_MAX_LEVELS];
        Laplacian3x3Node* m_pLNode[PYRAMID_MAX_LEVELS];
        ScaleImageNode*   m_pLSNode[PYRAMID_MAX_LEVELS];

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        PyramidNode(Graph* pGraph,
                    Image* pSrcImage, Pyramid* pDstGaussian, Pyramid* pDstLaplacian);
        ~PyramidNode() {}
    };
*/

/*
    class VX_CLASS ScaleImageNode : public Node {

        Image*           m_pSrcImage;
        Image*           m_pDstImage;
        Enumerant* m_pType;
        Array*          m_pLocalData;
        Size* m_pLocalDataSize;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        ScaleImageNode(Graph* pGraph,
                       Image* pSrcImage, Image* pDstImage, Enumerant* pType);
        ~ScaleImageNode() {}

        // setters
        void setSrcImage(Image* pImage)               { m_pSrcImage = pImage;  setDirty(); }
        void setDstImage(Image* pImage)               { m_pDstImage = pImage;  setDirty(); }
        void setType    (Enumerant* pType)      { m_pType     = pType;   setDirty(); }
    };
*/

    class VX_CLASS Sobel3x3Node : public Node {

        Image*      m_pSrcImage;
        Image*      m_pDstGradX;
        Image*      m_pDstGradY;
        BorderMode* m_pBorderMode;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        Sobel3x3Node(Graph* pGraph,
                     Image* pSrcImage, Image* pDstGradX, Image* pDstGradY,
                     BorderMode* pBorderMode = NULL);
        ~Sobel3x3Node() {}

        // setters
        void setSrcImage   (Image* pImage)           { m_pSrcImage   = pImage;      setDirty(); }
        void setOutputGradX(Image* pImage)           { m_pDstGradX   = pImage;      setDirty(); }
        void setOutputGradY(Image* pImage)           { m_pDstGradY   = pImage;      setDirty(); }
        void setBorderMode (BorderMode* pBorderMode) { m_pBorderMode = pBorderMode; setDirty(); }
    };


    class VX_CLASS ThresholdNode : public Node {

        Image*               m_pSrcImage;
        Threshold<vx_uint8>* m_pThreshold;
        Image*               m_pDstImage;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        ThresholdNode(Graph* pGraph,
                      Image* pSrcImage, Threshold<vx_uint8>* pThreshold, Image* pDstImage);
        ~ThresholdNode() {}

        // setters
        void setSrcImage (Image* pImage)                { m_pSrcImage  = pImage;  setDirty(); }
        void setDstImage (Image* pImage)                { m_pDstImage  = pImage;  setDirty(); }
        void setThreshold(Threshold<vx_uint8>* pThresh) { m_pThreshold = pThresh; setDirty(); }
    };


    class VX_CLASS WarpPerspectiveNode : public Node {

        Image*              m_pSrcImage;
        Matrix<vx_float32>* m_pMatrix;
        Enumerant*    m_pInterpType;
        Image*              m_pDstImage;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        WarpPerspectiveNode(Graph* pGraph,
                            Image* pSrcImage, Matrix<vx_float32>* pMatrix, Enumerant* pInterpType, Image* pDstImage);
        ~WarpPerspectiveNode() {}

        // setters
        void setSrcImage  (Image* pImage)                { m_pSrcImage   = pImage;      setDirty(); }
        void setDstImage  (Image* pImage)                { m_pDstImage   = pImage;      setDirty(); }
        void setMatrix    (Matrix<vx_float32>* pMatrix)  { m_pMatrix     = pMatrix;     setDirty(); }
        void setInterpType(Enumerant* pInterpType) { m_pInterpType = pInterpType; setDirty(); }
    };


    class VX_CLASS WarpAffineNode : public Node {

        Image*              m_pSrcImage;
        Matrix<vx_float32>* m_pMatrix;
        Enumerant*    m_pInterpType;
        Image*              m_pDstImage;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        WarpAffineNode(Graph* pGraph,
                       Image* pSrcImage, Matrix<vx_float32>* pMatrix, Enumerant* pInterpType, Image* pDstImage);
        ~WarpAffineNode() {}

        // setters
        void setSrcImage  (Image* pImage)                { m_pSrcImage   = pImage;      setDirty(); }
        void setDstImage  (Image* pImage)                { m_pDstImage   = pImage;      setDirty(); }
        void setMatrix    (Matrix<vx_float32>* pMatrix)  { m_pMatrix     = pMatrix;     setDirty(); }
        void setInterpType(Enumerant* pInterpType) { m_pInterpType = pInterpType; setDirty(); }
    };

    //---------------------------------------------------------------------------
    // debug nodes
    //---------------------------------------------------------------------------


    class VX_CLASS FReadImageNode : public Node {

        Image*  m_pImage;
        String* m_pFilename;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        FReadImageNode(Graph* pGraph, Image* pImage, String* pFilename);
        ~FReadImageNode() {}

        // setters
        void setImage   (Image* pImage)     { m_pImage    = pImage;    setDirty(); }
        void setFilename(String* pFilename) { m_pFilename = pFilename; setDirty(); }
    };


    class VX_CLASS FWriteImageNode : public Node {

        Image*  m_pImage;
        String* m_pFilename;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        FWriteImageNode(Graph* pGraph, Image* pImage, String* pFilename);
        ~FWriteImageNode() {}

        // setters
        void setImage   (Image* pImage)     { m_pImage    = pImage;    setDirty(); }
        void setFilename(String* pFilename) { m_pFilename = pFilename; setDirty(); }
    };


    class VX_CLASS CopyImageNode : public Node {

        Image* m_pSrcImage;
        Image* m_pDstImage;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        CopyImageNode(Graph* pGraph,
                      Image* pSrcImage, Image* pDstImage);
        ~CopyImageNode() {}
    };


    class VX_CLASS CopyArrayNode : public Node {

        Array* m_pSrcArray;
        Array* m_pDstArray;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        CopyArrayNode(Graph* pGraph,
                      Array* pSrcArray, Array* pDstArray);
        ~CopyArrayNode() {}
    };

    //---------------------------------------------------------------------------
    // extras nodes
    //---------------------------------------------------------------------------

    class VX_CLASS EuclideanNonMaxSuppressionHarrisNode : public Node {

        Image*              m_pSrcImage;
        Float32* m_pThreshold;
        Float32* m_pRadius;
        Image*              m_pDstImage;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        EuclideanNonMaxSuppressionHarrisNode(Graph* pGraph,
                                       Image* pSrcImage, Float32* pThreshold, Float32* pRadius,
                                       Image* pDstImage);
        ~EuclideanNonMaxSuppressionHarrisNode() {}
    };


    class VX_CLASS NonMaxSuppressionNode : public Node {

        Image* m_pSrcImageMag;
        Image* m_pSrcImageAng;
        Image* m_pDstImageEdge;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        NonMaxSuppressionNode(Graph* pGraph,
                              Image* pSrcImageMag, Image* pSrcImageAng, Image* pDstImageEdge);
        ~NonMaxSuppressionNode() {}
    };


    class VX_CLASS Laplacian3x3Node : public Node {

        Image*      m_pSrcImage;
        Image*      m_pDstImage;
        BorderMode* m_pBorderMode;

    protected:
        virtual vx_status defineNode(NodeProperties *pProperties);
        virtual vx_status initialize()   { return VX_SUCCESS; }
        virtual vx_status deinitialize() { return VX_SUCCESS; }

    public:
        Laplacian3x3Node(Graph* pGraph,
                         Image* pSrcImage, Image* pDstImage,
                         BorderMode* pBorderMode = NULL);
        ~Laplacian3x3Node() {}
    };

};  // namespace OpenVX

#endif

