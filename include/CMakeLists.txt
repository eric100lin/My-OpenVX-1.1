#
# Copyright (c) 2011-2014 The Khronos Group Inc.
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
#

install( DIRECTORY VX DESTINATION include )

set(OPENVX_STD_HEADER_BASENAMES
    vx.h vxu.h vx_vendors.h vx_types.h vx_kernels.h vx_api.h vx_nodes.h
    )

set(OPENVX_STD_HEADERS "")
set(OPENVX_STD_HEADERS_ABSPATH "")
foreach(h ${OPENVX_STD_HEADER_BASENAMES})
  set(OPENVX_STD_HEADERS ${OPENVX_STD_HEADERS} VX/${h})
  set(OPENVX_STD_HEADERS_ABSPATH ${OPENVX_STD_HEADERS_ABSPATH} ${CMAKE_SOURCE_DIR}/include/VX/${h})
endforeach()

set(OPENVX_HEADERS_DESTDIR "${CMAKE_BINARY_DIR}/packages")

set(OPENVX_STD_HEADERS_PACKAGE_NAME "openvx-standard-headers-${OPENVX_VERSION}.tar.bz2")

if (UNIX)
  add_custom_command(
      OUTPUT ${OPENVX_HEADERS_DESTDIR}/${OPENVX_STD_HEADERS_PACKAGE_NAME}
      COMMAND mkdir -p ${OPENVX_HEADERS_DESTDIR} && echo Create package ${OPENVX_HEADERS_DESTDIR}/${OPENVX_STD_HEADERS_PACKAGE_NAME} && tar -cjf ${OPENVX_HEADERS_DESTDIR}/${OPENVX_STD_HEADERS_PACKAGE_NAME} -C ${CMAKE_SOURCE_DIR}/include ${OPENVX_STD_HEADERS}
      DEPENDS ${OPENVX_STD_HEADERS_ABSPATH}
      )
  set(_INCLUDE_TO_ALL "")
  if (BUILD_PACKAGES)
    set(_INCLUDE_TO_ALL "ALL")
  endif()
  add_custom_target(openvx-standard-headers-package ${_INCLUDE_TO_ALL} DEPENDS ${OPENVX_HEADERS_DESTDIR}/${OPENVX_STD_HEADERS_PACKAGE_NAME})
endif()
