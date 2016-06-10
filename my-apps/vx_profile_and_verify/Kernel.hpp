#ifndef _KERNEL_HPP_
#define _KERNEL_HPP_
#include "vx.hpp"

namespace OpenVX
{
	class Kernel
	{
	private:
		vx_kernel m_kernel;
		std::string m_kernel_name;

		Kernel(std::string kernel_name);
		
	public:
		~Kernel();

		const char *getKernelName() const;

		vx_kernel getVxKernel(Context &context);
		
		static Kernel *getKernel(vx_kernel_e kernel_e, enum Target target_e);
		
		static std::string getKernelNameOfType(vx_kernel_e kernel_e);

		static vx_kernel_e getKernelTypeFromName(std::string kernelName);

		static std::string getFullKernelName(vx_kernel_e kernel_e, enum Target target_e);

		static vx_kernel_e getKernelEnumFromName(std::string kernelName);

		static Target getTargetEnumFromName(std::string targetName);
	};

};

#endif