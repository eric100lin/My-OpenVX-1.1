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


include $(PRELUDE)
TARGET := openvx-opencl
TARGETTYPE := dsmo
DEFFILE := openvx-target.def
CSOURCES = $(call all-c-files)
IDIRS += $(HOST_ROOT)/$(OPENVX_SRC)/include $(HOST_ROOT)/debug
SHARED_LIBS += openvx
DEFS += VX_CL_SOURCE_DIR="\"$(HOST_ROOT)/kernels/opencl\""
ifeq ($(TARGET_BUILD),debug)
# This is to use the local headers instead of system defined ones it's temporary
DEFS += VX_INCLUDE_DIR="\"$(HOST_ROOT)/include\""
endif
ifneq (,$(findstring EXPERIMENTAL_USE_OPENCL,$(SYSDEFS)))
USE_OPENCL:=true
else
SKIPBUILD:=1
endif
include $(FINALE)

