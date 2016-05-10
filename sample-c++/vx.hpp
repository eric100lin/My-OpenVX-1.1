/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
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

#ifndef _VX_HPP_
#define _VX_HPP_

#undef VX_CLASS
#if defined(WIN32) || defined(CYGWIN)
#if defined(VX_EXPORTING)
// #pragma message("EXPORTING")
#define VX_CLASS __declspec(dllexport)
#else
// #pragma message("IMPORTING")
#define VX_CLASS __declspec(dllimport)
#endif
#else
// #warning "No link modifiers"
#define VX_CLASS
#endif

// Forward Declarations (ordering matches inclusion order below)
namespace OpenVX {
    class Reference;
    class Parameter;
    class Array;
    class Convolution;
    class Distribution;
    class Graph;
    class Image;
    template <class T> class LUT;
    template <class T> class Matrix;
    class Node;
    class Pyramid;
    class Remap;
    class Scalar;
    class Char; class Bool; class Enumerant; class FourCC; class Size;
    class UInt8; class Int8;
    class UInt16; class Int16;
    class UInt32; class Int32;
    class UInt64; class Int64;
    class Float32; class Float64;
    class String;
#if defined(EXPERIMENTAL_USE_TARGET)
    class Target;
#endif
    template <class T> class Threshold;
    template <class T> class Delay;
    class Context;
};

#include <VX/vx.h>
/* TODO: remove vx_compatibility.h after transition period */
#include <VX/vx_compatibility.h>
#include <VX/vx_khr_tiling.h>
#include <VX/vx_khr_node_memory.h>
#if defined(EXPERIMENTAL_USE_TARGET)
#include <VX/vx_ext_target.h>
#endif

// alphabetical ordering, except where noted
#include <Reference.hpp>    // must be first since everything else is derived from it
#include <Parameter.hpp>    // must be early since others rely on it
#include <Array.hpp>
#include <Convolution.hpp>
#include <Distribution.hpp>
#include <Graph.hpp>
#include <Image.hpp>
#include <LUT.hpp>
#include <Matrix.hpp>
#include <Node.hpp>
#include <Pyramid.hpp>
#include <Remap.hpp>
#include <Scalar.hpp>
#include <String.hpp>
#if defined(EXPERIMENTAL_USE_TARGET)
#include <Target.hpp>
#endif
#include <Threshold.hpp>
#include <Delay.hpp>        // must be nearly last since it needs to wrap other objects
#include <Context.hpp>      // needs to be last since it creates all other objects

#endif

