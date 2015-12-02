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

OPENVX_STD_HEADER_BASENAMES = \
 vx.h vxu.h vx_vendors.h vx_types.h vx_kernels.h vx_api.h vx_nodes.h

OPENVX_STD_HEADERS = $(patsubst %, VX/%, $(OPENVX_STD_HEADER_BASENAMES))

# This is just the default destination, expected to be overridden.
OPENVX_HEADERS_DESTDIR = $(HOST_ROOT)/out

OPENVX_STD_HEADERS_PACKAGE_NAME = openvx-standard-headers-$(VERSION).tar.bz2

# Only do this for the same host environments where the concerto
# "tar" package type is available.
ifneq ($(filter $(HOST_OS),LINUX CYGWIN DARWIN),)

.PHONY: openvx-standard-headers-package
openvx-standard-headers-package: \
 $(OPENVX_HEADERS_DESTDIR)/$(OPENVX_STD_HEADERS_PACKAGE_NAME)

$(OPENVX_HEADERS_DESTDIR)/$(OPENVX_STD_HEADERS_PACKAGE_NAME): \
 $(patsubst %, $(HOST_ROOT)/include/%, $(OPENVX_STD_HEADERS))
	@$(MKDIR) $(OPENVX_HEADERS_DESTDIR)
	@tar -cjf $@ -C $(HOST_ROOT)/include $(OPENVX_STD_HEADERS)

endif # Host environments.
