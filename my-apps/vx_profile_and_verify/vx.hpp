#ifndef _VX_HPP_
#define _VX_HPP_

#include <VX/vx.h>
#if defined(EXPERIMENTAL_USE_TARGET)
#include <VX/vx_ext_target.h>
#endif
#include "vxUtils.h"

#include <stdlib.h>
#include <stdarg.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <map>
#include <iterator>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace OpenVX
{
	enum Target { TARGET_C_MODEL=0, TARGET_OPENCL=1, TARGET_HEXAGON=2, TARGET_OPENMP=3, TARGET_FASTCV=4, TARGET_UNKNOWN };
	
	class Context;
	class Graph;
	class Kernel;
	class Node;
	class Image;
	class VirtualImage;
	class LogStream;
}

#include "Context.hpp"
#include "Graph.hpp"
#include "Image.hpp"
#include "Kernel.hpp"
#include "LogStream.h"
#include "Node.hpp"
#include "OpenCV_data_translator.hpp"
#include "VirtualImage.hpp"

#endif