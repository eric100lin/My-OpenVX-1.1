/*
 * Copyright (c) 2016 National Tsing Hua University.
 * \brief The Hexagon OpenVX Kernel Interfaces
 * \author Tzu-Hsiang Lin <thlin@pllab.cs.nthu.edu.tw>
 */

#ifndef _VX_TAGET_HEXAGON_INTERFACE_H_
#define _VX_TAGET_HEXAGON_INTERFACE_H_

#include <VX/vx_helper.h>

extern vx_kernel_description_t not_kernel;
extern vx_kernel_description_t box3x3_kernel;
extern vx_kernel_description_t gaussian3x3_kernel;
extern vx_kernel_description_t and_kernel;
extern vx_kernel_description_t xor_kernel;
extern vx_kernel_description_t add_kernel;
extern vx_kernel_description_t subtract_kernel;
extern vx_kernel_description_t threshold_kernel;

#endif
