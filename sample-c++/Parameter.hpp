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

#ifndef _VX_PARAMETER_HPP_
#define _VX_PARAMETER_HPP_

namespace OpenVX {

    class VX_CLASS Parameter : public Reference {

        // a parameter is isomorphic to a single "edge" (or fanning set of edges) in the graph
        // since parameters may not appear as outputs from more than one node. the data on
        // that edge is invalid until the upstream node has executed
        bool m_valid;

    public:
        Parameter(Context* pContext, vx_reference ref) : Reference(pContext, ref)  { m_valid = false; }
        Parameter(Context* pContext)                   : Reference(pContext)       { m_valid = false; }
        ~Parameter() {}

        void setValid(bool valid)                   { m_valid = valid; }
        void markValid()                            { m_valid = true;  }
        void markInvalid()                          { m_valid = false; }
        bool isValid()                              { return m_valid;  }
    };


    typedef enum PortDirection_ {
        pdIn,
        pdOut,
        pdNonFlowing,
        pdUnknown
    } PortDirection;


    // the port via which a parameter flows into or out of a node: basically just a parameter and a direction

    class VX_CLASS Port {

        friend class Node;

        Parameter*    m_pParameter;
        PortDirection m_direction;

    public:
        Port () {
            m_pParameter = NULL;
            m_direction  = pdUnknown;
        }

//         Port (Parameter* pParam, PortDirection direction) {
//             m_pParameter = pParam;
//             m_direction  = direction;
//         }

        ~Port() {}

        void set(Parameter* pParam, PortDirection direction) {
            m_pParameter = pParam;
            m_direction  = direction;
        }

        PortDirection direction() { return m_direction; }
    };


    // BUGBUG move this
    class VX_CLASS BorderMode : public Parameter {
    public:
        vx_border_t m_borderMode;

        BorderMode(Context* pContext) : Parameter(pContext)
        {
            m_borderMode.mode = VX_BORDER_REPLICATE;
            m_borderMode.constant_value.U8 = 0;
            // border modes are valid from the get-go
            markValid();
        }

        BorderMode(Context* pContext, BorderMode* pBM) : Parameter(pContext)
        {
            if (pBM) {
                m_borderMode = pBM->m_borderMode;
            }
            else {
                m_borderMode.mode = VX_BORDER_REPLICATE;
                m_borderMode.constant_value.U8 = 0;
            }
            // border modes are valid from the get-go
            markValid();
        }

        BorderMode(Context* pContext, vx_enum mode, vx_uint32 constant_value) : Parameter(pContext)
        {
            m_borderMode.mode = mode;
            m_borderMode.constant_value.U8 = constant_value;
            // border modes are valid from the get-go
            markValid();
        }
    };

}

#endif

