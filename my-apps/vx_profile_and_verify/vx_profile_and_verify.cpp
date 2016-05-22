#include <iostream>
#include "vx.hpp"
#include "Application.hpp"
#define N_TIMES 100
using namespace cv;
using namespace OpenVX;

int main(int argc, char **argv)
{
	Context context;
	context.selfTest();
	std::cout << std::endl;

	Application *apps[] = 
	{
		new AppOneIOneO(context, VX_KERNEL_NOT),
		new AppTwoIOneO(context, VX_KERNEL_AND),
		new AppTwoIOneO(context, VX_KERNEL_XOR),
		new AppOneIOneO(context, VX_KERNEL_BOX_3x3),
		new AppOneIOneO(context, VX_KERNEL_GAUSSIAN_3x3),
		//new AppTwoIOneO(context, VX_KERNEL_OR),			//NO fcv function
		//new AppTableLookup(context),
		//new AppHistogram(context),
	};
	enum Target targets[] = 
	{ 
		TARGET_C_MODEL, TARGET_OPENCL, TARGET_HEXAGON 
	};
	int n_apps = sizeof(apps) / sizeof(apps[0]);
	int n_targets = sizeof(targets) / sizeof(targets[0]);
	
	std::cout << "Process and Verify:" << std::endl;
	for (int i = 0; i < n_apps; i++)
	{
		std::cout << "apps[" << i << "]:" << std::endl;
		apps[i]->prepareInput();
		for (int t = 1; t < n_targets; t++)
		{
			std::cout << " Target[" << t << "]: " << 
				apps[i]->getKernelFullName(targets[t]) << std::endl;
			
			apps[i]->setup();
			
			apps[i]->process(targets[t]);
			
			if (!apps[i]->verify())
				std::cout << "\tverify fail" << std::endl;
			else
				std::cout << "\tverify success" << std::endl;
			
			apps[i]->release();
		}
	}
	std::cout << std::endl;
	
	std::cout << "Profile " << n_apps << " apps over " << N_TIMES << " loop:" << std::endl;
	for (int i = 0; i < n_apps; i++)
	{
		std::cout << "apps[" << i << "]:" << std::endl;
		for (int t = 0; t < n_targets; t++)
		{
			std::cout << " Target[" << t << "]: " << 
				apps[i]->getKernelFullName(targets[t]) << std::endl;
			
			apps[i]->setup();
			
			apps[i]->profiling(N_TIMES, targets[t]);
			
			apps[i]->release();
		}
	}
	std::cout << std::endl;

	for (int i = 0; i < n_apps; i++)
		delete apps[i];
	
	std::cout << argv[0] << " done!!" << endl;
	//system("PAUSE");
	return 0;
}
