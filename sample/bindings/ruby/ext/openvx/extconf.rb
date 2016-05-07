#!/usr/bin/ruby -w

# Copyright (c) 2012-2016 The Khronos Group Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and/or associated documentation files (the
# "Materials"), to deal in the Materials without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Materials, and to
# permit persons to whom the Materials are furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Materials.
#
# THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.

require 'mkmf'
$preload = ["openvx"]
dir_config("openvx")
openvx_root = "#{ENV['OPENVX_ROOT']}"
openvx_out = "#{ENV['OPENVX_OUT']}"
openvx_inc = File.join(openvx_root,"include")
find_header("VX/vx.h",openvx_inc)
find_library("openvx","vxCreateContext",openvx_out)
find_library("vxu","vxuColorConvert",openvx_out)
$defs.push("-D_LITTLE_ENDIAN_") unless $defs.include?("-D_LITTLE_ENDIAN_")
$defs.push("-I#{ENV['OPENVX_ROOT']}/include")
$defs.push("-L#{ENV['OPENVX_OUT']}")
create_makefile("openvx/openvx")

