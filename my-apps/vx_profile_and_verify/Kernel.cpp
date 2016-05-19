#include "Kernel.hpp"

using namespace OpenVX;
using namespace std;

Kernel::Kernel(string kernel_name)
{
	m_kernel_name = kernel_name;
}

Kernel::~Kernel()
{
	vx_status status = vxReleaseKernel(&m_kernel);
	ERROR_CHECK(status);
}

const char *Kernel::getKernelName() const
{
	return m_kernel_name.c_str();
}

vx_kernel Kernel::getVxKernel(Context &context)
{
	m_kernel = vxGetKernelByName(context.getVxContext(), getKernelName());
	GET_STATUS_CHECK(m_kernel);
	return m_kernel;
}

static struct { const char * name; Target value; } t_table_constants[] = {
	{ "khronos.c_model", TARGET_C_MODEL },
	{ "pc.opencl", TARGET_OPENCL },
	{ "khronos.openmp", TARGET_OPENMP },
	{ "android.hexagon", TARGET_HEXAGON },
	{ NULL, TARGET_UNKNOWN }
};

static struct { const char * name; vx_kernel_e value; } k_table_constants[] = {
	{ "org.khronos.openvx.color_convert", VX_KERNEL_COLOR_CONVERT },
	{ "org.khronos.openvx.channel_extract", VX_KERNEL_CHANNEL_EXTRACT },
	{ "org.khronos.openvx.channel_combine", VX_KERNEL_CHANNEL_COMBINE },
	{ "org.khronos.openvx.sobel_3x3", VX_KERNEL_SOBEL_3x3 },
	{ "org.khronos.openvx.magnitude", VX_KERNEL_MAGNITUDE },
	{ "org.khronos.openvx.phase", VX_KERNEL_PHASE },
	{ "org.khronos.openvx.scale_image", VX_KERNEL_SCALE_IMAGE },
	{ "org.khronos.openvx.table_lookup", VX_KERNEL_TABLE_LOOKUP },
	{ "org.khronos.openvx.histogram", VX_KERNEL_HISTOGRAM },
	{ "org.khronos.openvx.equalize_histogram", VX_KERNEL_EQUALIZE_HISTOGRAM },
	{ "org.khronos.openvx.absdiff", VX_KERNEL_ABSDIFF },
	{ "org.khronos.openvx.mean_stddev", VX_KERNEL_MEAN_STDDEV },
	{ "org.khronos.openvx.threshold", VX_KERNEL_THRESHOLD },
	{ "org.khronos.openvx.dilate_3x3", VX_KERNEL_DILATE_3x3 },
	{ "org.khronos.openvx.erode_3x3", VX_KERNEL_ERODE_3x3 },
	{ "org.khronos.openvx.median_3x3", VX_KERNEL_MEDIAN_3x3 },
	{ "org.khronos.openvx.box_3x3", VX_KERNEL_BOX_3x3 },
	{ "org.khronos.openvx.gaussian_3x3", VX_KERNEL_GAUSSIAN_3x3 },
	{ "org.khronos.openvx.custom_convolution", VX_KERNEL_CUSTOM_CONVOLUTION },
	{ "org.khronos.openvx.gaussian_pyramid", VX_KERNEL_GAUSSIAN_PYRAMID },
	{ "org.khronos.openvx.accumulate", VX_KERNEL_ACCUMULATE },
	{ "org.khronos.openvx.accumulate_weighted", VX_KERNEL_ACCUMULATE_WEIGHTED },
	{ "org.khronos.openvx.accumulate_square", VX_KERNEL_ACCUMULATE_SQUARE },
	{ "org.khronos.openvx.minmaxloc", VX_KERNEL_MINMAXLOC },
	{ "org.khronos.openvx.convertdepth", VX_KERNEL_CONVERTDEPTH },
	{ "org.khronos.openvx.canny_edge_detector", VX_KERNEL_CANNY_EDGE_DETECTOR },
	{ "org.khronos.openvx.and", VX_KERNEL_AND },
	{ "org.khronos.openvx.or", VX_KERNEL_OR },
	{ "org.khronos.openvx.xor", VX_KERNEL_XOR },
	{ "org.khronos.openvx.not", VX_KERNEL_NOT },
	{ "org.khronos.openvx.multiply", VX_KERNEL_MULTIPLY },
	{ "org.khronos.openvx.add", VX_KERNEL_ADD },
	{ "org.khronos.openvx.subtract", VX_KERNEL_SUBTRACT },
	{ "org.khronos.openvx.warp_affine", VX_KERNEL_WARP_AFFINE },
	{ "org.khronos.openvx.warp_perspective", VX_KERNEL_WARP_PERSPECTIVE },
	{ "org.khronos.openvx.harris_corners", VX_KERNEL_HARRIS_CORNERS },
	{ "org.khronos.openvx.fast_corners", VX_KERNEL_FAST_CORNERS },
	{ "org.khronos.openvx.optical_flow_pyr_lk", VX_KERNEL_OPTICAL_FLOW_PYR_LK },
	{ "org.khronos.openvx.remap", VX_KERNEL_REMAP },
	{ "org.khronos.openvx.halfscale_gaussian", VX_KERNEL_HALFSCALE_GAUSSIAN },
	{ "org.khronos.openvx.laplacian_pyramid", VX_KERNEL_LAPLACIAN_PYRAMID },
	{ "org.khronos.openvx.laplacian_reconstruct", VX_KERNEL_LAPLACIAN_RECONSTRUCT },
	//{ "", VX_KERNEL_NON_LINEAR_FILTER },
	{ NULL, VX_KERNEL_MAX_1_0 }
};

std::string Kernel::getFullKernelName(vx_kernel_e kernel_e, enum Target target_e)
{
	for (vx_uint32 t = 0; t_table_constants[t].name; t++) 
	{
		if (t_table_constants[t].value == target_e) 
		{
			for (vx_uint32 k = 0; k_table_constants[k].name; k++) 
			{
				if (k_table_constants[k].value == kernel_e) {
					stringstream ss;
					ss << t_table_constants[t].name << ":" << k_table_constants[k].name;
					return ss.str();
				}
			}
		}
	}
	ReportError("Kernel not found");
	return std::string("");
}

Kernel *Kernel::getKernel(vx_kernel_e kernel_e, enum Target target_e)
{
	std::string fullName = getFullKernelName(kernel_e, target_e);
	return new Kernel(fullName.c_str());
}
