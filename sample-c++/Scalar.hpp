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

#ifndef _VX_SCALAR_HPP_
#define _VX_SCALAR_HPP_

#include <vx.hpp>
#include <assert.h>

namespace OpenVX {

    class VX_CLASS Scalar : public Parameter {

        friend class Context;

    protected:
        /*! \brief Maps to vxCreateScalar */
        Scalar(Context* pContext, vx_enum type, void *v);

    public:
        ~Scalar();     // vxReleaseScalar
        vx_enum type();// vxQueryScalar
    };

    class VX_CLASS Char : public Scalar {
        friend class Context;
    protected:
        Char(Context *pContext, vx_char v) : Scalar(pContext, VX_TYPE_CHAR, &v) {}
    public:
        vx_char AccessValue();
        void CommitValue(vx_char v);
    };

    class VX_CLASS Bool : public Scalar {
        friend class Context;
    protected:
        Bool(Context *pContext, vx_bool v) : Scalar(pContext, VX_TYPE_BOOL, &v) {}
    public:
        vx_bool AccessValue();
        void CommitValue(vx_bool v);
    };

    class VX_CLASS Enumerant : public Scalar {
        friend class Context;
    protected:
        Enumerant(Context *pContext, vx_enum v) : Scalar(pContext, VX_TYPE_ENUM, &v) {}
    public:
        vx_enum AccessValue();
        void CommitValue(vx_enum v);
    };

    class VX_CLASS FourCC : public Scalar {
        friend class Context;
    protected:
        FourCC(Context *pContext, vx_df_image v) : Scalar(pContext, VX_TYPE_DF_IMAGE, &v) {}
    public:
        vx_df_image AccessValue();
        void CommitValue(vx_df_image v);
    };

    class VX_CLASS Size : public Scalar {
        friend class Context;
    protected:
        Size(Context *pContext, vx_size v) : Scalar(pContext, VX_TYPE_SIZE, &v) {}
    public:
        vx_size AccessValue();
        void CommitValue(vx_size v);
    };

    class VX_CLASS UInt8 : public Scalar {
        friend class Context;
    protected:
        UInt8(Context *pContext, vx_uint8 v) : Scalar(pContext, VX_TYPE_UINT8, &v) {}
    public:
        vx_uint8 AccessValue();
        void CommitValue(vx_uint8 v);
    };

    class VX_CLASS Int8 : public Scalar {
        friend class Context;
    protected:
        Int8(Context *pContext, vx_int8 v) : Scalar(pContext, VX_TYPE_INT8, &v) {}
    public:
        vx_int8 AccessValue();
        void CommitValue(vx_int8 v);
    };

    class VX_CLASS UInt16 : public Scalar {
        friend class Context;
    protected:
        UInt16(Context *pContext, vx_uint16 v) : Scalar(pContext, VX_TYPE_UINT16, &v) {}
    public:
        vx_uint16 AccessValue();
        void CommitValue(vx_uint16 v);
    };

    class VX_CLASS Int16 : public Scalar {
        friend class Context;
    protected:
        Int16(Context *pContext, vx_int16 v) : Scalar(pContext, VX_TYPE_INT16, &v) {}
    public:
        vx_int16 AccessValue();
        void CommitValue(vx_int16 v);
    };

    class VX_CLASS UInt32 : public Scalar {
        friend class Context;
    protected:
        UInt32(Context *pContext, vx_uint32 v) : Scalar(pContext, VX_TYPE_UINT32, &v) {}
    public:
        vx_uint32 AccessValue();
        void CommitValue(vx_uint32 v);
    };

    class VX_CLASS Int32 : public Scalar {
        friend class Context;
    protected:
        Int32(Context *pContext, vx_int32 v) : Scalar(pContext, VX_TYPE_INT32, &v) {}
    public:
        vx_int32 AccessValue();
        void CommitValue(vx_int32 v);
    };

    class VX_CLASS UInt64 : public Scalar {
        friend class Context;
    protected:
        UInt64(Context *pContext, vx_uint64 v) : Scalar(pContext, VX_TYPE_UINT64, &v) {}
    public:
        vx_uint64 AccessValue();
        void CommitValue(vx_uint64 v);
    };

    class VX_CLASS Int64 : public Scalar {
        friend class Context;
    protected:
        Int64(Context *pContext, vx_int64 v) : Scalar(pContext, VX_TYPE_INT64, &v) {}
    public:
        vx_int64 AccessValue();
        void CommitValue(vx_int64 v);
    };


    class VX_CLASS Float32 : public Scalar {
        friend class Context;
    protected:
        Float32(Context *pContext, vx_float32 v) : Scalar(pContext, VX_TYPE_FLOAT32, &v) {}
    public:
        vx_float32 AccessValue();
        void CommitValue(vx_float32 v);
    };

    class VX_CLASS Float64 : public Scalar {
        friend class Context;
    protected:
        Float64(Context *pContext, vx_float64 v) : Scalar(pContext, VX_TYPE_FLOAT64, &v) {}
    public:
        vx_float64 AccessValue();
        void CommitValue(vx_float64 v);
    };
}

#endif

