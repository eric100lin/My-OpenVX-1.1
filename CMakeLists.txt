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

#
# Global setup file for OpenVX CMake
#
cmake_minimum_required(VERSION 2.8.9)

file(READ "${CMAKE_SOURCE_DIR}/VERSION" OPENVX_VERSION)
string(STRIP "${OPENVX_VERSION}" OPENVX_VERSION)
set(OPENVX_VERSION ${OPENVX_VERSION} CACHE INTERNAL "" FORCE)

message(STATUS "===== OpenVX version: ${OPENVX_VERSION} =====")

# CMAKE variables
set( CMAKE_MODULE_PATH
     ${CMAKE_MODULE_PATH}
     "${CMAKE_SOURCE_DIR}/cmake_utils/"
)

include( cmake_utils/CMakeFuncs.txt )

if (CMAKE_TOOLCHAIN_FILE)
	## Do nothing!!
else (CMAKE_TOOLCHAIN_FILE)
	if (WIN32)
		include( CMake_windows_tools )
	elseif (ANDROID)
		include( CMake_android_tools )
	else ()
		include( CMake_linux_tools )
	endif ()
endif (CMAKE_TOOLCHAIN_FILE)

project (OpenVX)

# Supported configurations
set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
  )
set(CMAKE_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES} CACHE STRING "Available build configurations." FORCE)

option( OPENVX_USE_TILING OFF )
option( EXPERIMENTAL_USE_NODE_MEMORY OFF )
option( EXPERIMENTAL_USE_OPENMP OFF )
option( EXPERIMENTAL_USE_OPENCL OFF )
option( EXPERIMENTAL_USE_DOT OFF )
option( EXPERIMENTAL_USE_XML OFF )
option( EXPERIMENTAL_USE_TARGET OFF )
option( EXPERIMENTAL_USE_VARIANTS OFF )
option( EXPERIMENTAL_USE_S16 OFF )
option( EXPERIMENTAL_PLATFORM_SUPPORTS_16_FLOAT OFF )

if (UNIX OR ANDROID)
    add_definitions( -D_XOPEN_SOURCE=700 -D_BSD_SOURCE=1 -D_GNU_SOURCE=1 )
    if (NOT (CYGWIN OR ANDROID))
        set( EXPERIMENTAL_USE_OPENMP ON )
    endif (NOT (CYGWIN OR ANDROID))
    set( EXPERIMENTAL_USE_DOT ON )
endif (UNIX OR ANDROID)

add_definitions( -DOPENVX_BUILDING ) 
if (OPENVX_USE_SMP)
    add_definitions( -DOPENVX_USE_SMP )
endif (OPENVX_USE_SMP)
if (OPENVX_USE_TILING)
    add_definitions( -DOPENVX_USE_TILING )
endif (OPENVX_USE_TILING)
if (EXPERIMENTAL_USE_NODE_MEMORY)
    add_definitions( -DEXPERIMENTAL_USE_NODE_MEMORY )
endif (EXPERIMENTAL_USE_NODE_MEMORY)
if (EXPERIMENTAL_USE_OPENMP)
    add_definitions( -DEXPERIMENTAL_USE_OPENMP )
endif (EXPERIMENTAL_USE_OPENMP)
if (EXPERIMENTAL_USE_OPENCL)
    add_definitions( -DEXPERIMENTAL_USE_OPENCL )
endif (EXPERIMENTAL_USE_OPENCL)
if (EXPERIMENTAL_USE_HEXAGON)
    add_definitions( -DEXPERIMENTAL_USE_HEXAGON )
endif (EXPERIMENTAL_USE_HEXAGON)
if (EXPERIMENTAL_USE_DOT)
    add_definitions( -DEXPERIMENTAL_USE_DOT )
endif (EXPERIMENTAL_USE_DOT)
if (EXPERIMENTAL_USE_XML)
	find_package(PkgConfig)
	pkg_check_modules(PC_LIBXML QUIET libxml-2.0)
	set(LIBXML2_DEFINITIONS ${PC_LIBXML_CFLAGS_OTHER})

	find_path(LIBXML2_INCLUDE_DIR libxml/xpath.h
			  HINTS ${PC_LIBXML_INCLUDEDIR} ${PC_LIBXML_INCLUDE_DIRS}
			  PATH_SUFFIXES libxml2 )

	find_library(LIBXML2_LIBRARY NAMES xml2 libxml2
				 HINTS ${PC_LIBXML_LIBDIR} ${PC_LIBXML_LIBRARY_DIRS} )

	include(FindPackageHandleStandardArgs)
	# handle the QUIETLY and REQUIRED arguments and set LIBXML2_FOUND to TRUE
	# if all listed variables are TRUE
	find_package_handle_standard_args(LibXml2  DEFAULT_MSG
									  LIBXML2_LIBRARY LIBXML2_INCLUDE_DIR)

	mark_as_advanced(LIBXML2_INCLUDE_DIR LIBXML2_LIBRARY )

	set(LIBXML2_LIBRARIES ${LIBXML2_LIBRARY} )
	set(LIBXML2_INCLUDE_DIRS ${LIBXML2_INCLUDE_DIR} )
    add_definitions( -DEXPERIMENTAL_USE_XML )
endif (EXPERIMENTAL_USE_XML)
if (EXPERIMENTAL_USE_TARGET)
    add_definitions( -DEXPERIMENTAL_USE_TARGET )
endif (EXPERIMENTAL_USE_TARGET)
if (EXPERIMENTAL_USE_VARIANTS)
    add_definitions( -DEXPERIMENTAL_USE_VARIANTS )
endif (EXPERIMENTAL_USE_VARIANTS)
if (EXPERIMENTAL_USE_S16)
    add_definitions( -DEXPERIMENTAL_USE_S16 )
endif (EXPERIMENTAL_USE_S16)
if (EXPERIMENTAL_PLATFORM_SUPPORTS_16_FLOAT)
    add_definitions( -DEXPERIMENTAL_PLATFORM_SUPPORTS_16_FLOAT )
endif (EXPERIMENTAL_PLATFORM_SUPPORTS_16_FLOAT)

if (BUILD_X64)
    add_definitions( -DARCH_64 )
else (BUILD_X64)
    add_definitions( -DARCH_32 )
endif (BUILD_X64)

if (CMAKE_BUILD_TYPE EQUAL Debug)
    add_definitions( -DOPENVX_DEBUGGING )
endif (CMAKE_BUILD_TYPE EQUAL Debug)

if ( EXPERIMENTAL_USE_OPENCL AND NOT USE_POCL_OPENCL)
    if (DEFINED ENV{VX_OPENCL_INCLUDE_PATH})
        set( OPENCL_INCLUDE_PATH $ENV{VX_OPENCL_INCLUDE_PATH} )
        message( "Use OpenCL headers from: " ${OPENCL_INCLUDE_PATH} )
	else(DEFINED ENV{VX_OPENCL_INCLUDE_PATH})
	    set( OPENCL_INCLUDE_PATH )
	    message( "Use OpenCL headers from system default location, in order to set specific path: please set 'VX_OPENCL_INCLUDE_PATH' env. variable and reconfigure")
	endif (DEFINED ENV{VX_OPENCL_INCLUDE_PATH})
	if (DEFINED ENV{VX_OPENCL_LIB_PATH})
        set( OPENCL_LIB_PATH $ENV{VX_OPENCL_LIB_PATH} )
	    message( "Use OpenCL headers from: " ${OPENCL_LIB_PATH} )
	else(DEFINED ENV{VX_OPENCL_LIB_PATH})
	    message( "Use OpenCL libs from system default location, in order to set specific path: please set 'VX_OPENCL_LIB_PATH' env. variable and reconfigure")
    endif (DEFINED ENV{VX_OPENCL_LIB_PATH})
endif ( EXPERIMENTAL_USE_OPENCL AND NOT USE_POCL_OPENCL)

if ( ANDROID AND EXPERIMENTAL_USE_HEXAGON )
  #Check for VX_HEXAGON_LIB_PATH environment variable
  if (DEFINED ENV{VX_HEXAGON_LIB_PATH})
    set( HEXAGON_LIB_PATH $ENV{VX_HEXAGON_LIB_PATH} )
    message( "Use Hexagon library from: " ${HEXAGON_LIB_PATH} )
  else(DEFINED ENV{VX_HEXAGON_LIB_PATH})
    message(FATAL_ERROR "Please set 'VX_HEXAGON_LIB_PATH' env. variable and reconfigure")
  endif (DEFINED ENV{VX_HEXAGON_LIB_PATH})
  
  #Check for VX_HEXAGON_REMOTE_PATH environment variable
  if (DEFINED ENV{VX_HEXAGON_REMOTE_PATH})
    set( HEXAGON_REMOTE_PATH $ENV{VX_HEXAGON_REMOTE_PATH} )
    message( "Use Hexagon remote header and library from: " ${HEXAGON_REMOTE_PATH} )
  else(DEFINED ENV{VX_HEXAGON_REMOTE_PATH})
    message(FATAL_ERROR "Please set 'VX_HEXAGON_REMOTE_PATH' env. variable and reconfigure")
  endif (DEFINED ENV{VX_HEXAGON_REMOTE_PATH})
endif ( ANDROID AND EXPERIMENTAL_USE_HEXAGON)

if (CMAKE_TOOLCHAIN_FILE)
	## Do nothing!!
else(CMAKE_TOOLCHAIN_FILE)
	FIND_NUM_PROCESSORS()
endif(CMAKE_TOOLCHAIN_FILE)


if ( ${PROCESSOR_COUNT} LESS "1" )
	message(FATAL_ERROR "Num processors found is " ${PROCESSOR_COUNT})
endif ( ${PROCESSOR_COUNT} LESS "1" )
add_definitions( -DTARGET_NUM_CORES=${PROCESSOR_COUNT} )

# Framework IDE Folders Names
set_property( GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "" )
set_property( GLOBAL PROPERTY USE_FOLDERS ON )
set( DEBUG_FOLDER Debug )
set( EXAMPLES_FOLDER Examples )
set( MYAPPS_FOLDER my-apps )
set( HELPER_FOLDER Helper )
set( KERNELS_FOLDER Kernels )
set( LIBRARIES_FOLDER Libraries )
set( LIBRARIES_DEBUG_FOLDER ${LIBRARIES_FOLDER}/Debug )
set( LIBRARIES_EXTRAS_FOLDER ${LIBRARIES_FOLDER}/Extras )
set( SAMPLE_FOLDER Sample )
set( SAMPLE_TARGETS_FOLDER ${SAMPLE_FOLDER}/Targets )
set( SAMPLE_CPP_FOLDER SampleCPP )
set( TOOLS_FOLDER Tools )


if ( EXPERIMENTAL_USE_OPENCL AND USE_POCL_OPENCL )
	set( TARGET_NAME POCL-OpenCL )
	include_directories(BEFORE ${CMAKE_SOURCE_DIR}/libopencl-stub/include)
	add_library (${TARGET_NAME} STATIC ${CMAKE_SOURCE_DIR}/libopencl-stub/libopencl.c)
	set( OPENCL_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/libopencl-stub/include )
	install ( TARGETS ${TARGET_NAME} 
          RUNTIME DESTINATION bin
          ARCHIVE DESTINATION bin
          LIBRARY DESTINATION bin )
	set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${DEBUG_FOLDER} )
endif ( EXPERIMENTAL_USE_OPENCL AND USE_POCL_OPENCL )

add_subdirectory( debug )
add_subdirectory( examples )
add_subdirectory( helper )
add_subdirectory( kernels )
add_subdirectory( libraries )
add_subdirectory( sample )
add_subdirectory( sample-c++ )
add_subdirectory( tools )
add_subdirectory( include )
add_subdirectory( my-apps )
