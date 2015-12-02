# Copyright (c) 2015 The Khronos Group Inc.
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

# Make a bzip2 tarball containing just the standard headers
# (no extensions).

OPENVX_SAMPLE_DIR_NAMES = \
 cmake_utils concerto debug examples helper include kernels libraries raw sample scripts tools \
 Android.mk BUILD_DEFINES Build.py CMakeLists.txt LICENSE Makefile NEWKERNEL.txt README VERSION

OPENVX_SAMPLE_DIRS = $(patsubst %, %, $(OPENVX_SAMPLE_DIR_NAMES))

# This is just the default destination, expected to be overridden.
OPENVX_SAMPLE_DESTDIR = $(HOST_ROOT)/out

OPENVX_SAMPLE_PACKAGE_NAME = openvx_sample_$(VERSION).tar.bz2

# Only do this for the same host environments where the concerto
# "tar" package type is available.
ifneq ($(filter $(HOST_OS),LINUX CYGWIN DARWIN),)

.PHONY: openvx_sample_package
openvx_sample_package: \
 $(OPENVX_SAMPLE_DESTDIR)/$(OPENVX_SAMPLE_PACKAGE_NAME)

$(OPENVX_SAMPLE_DESTDIR)/$(OPENVX_SAMPLE_PACKAGE_NAME): \
 $(patsubst %, $(HOST_ROOT)/%, $(OPENVX_SAMPLE_DIRS))
	@$(MKDIR) $(OPENVX_SAMPLE_DESTDIR)
	@tar -cjf $@ -C $(HOST_ROOT) $(OPENVX_SAMPLE_DIRS)

endif # Host environments.
