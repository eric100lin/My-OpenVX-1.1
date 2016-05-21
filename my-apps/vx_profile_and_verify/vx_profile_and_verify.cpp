#include <iostream>
#include "vx.hpp"
#include "Application.hpp"
using namespace cv;
using namespace OpenVX;

int main(int argc, char **argv)
{
	Context context;
	context.selfTest();
	std::cout << std::endl;

	std::cout << "Profile and Verify:" << std::endl;
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
		TARGET_OPENCL, TARGET_HEXAGON 
	};
	int n_apps = sizeof(apps) / sizeof(apps[0]);
	int n_targets = sizeof(targets) / sizeof(targets[0]);
	for (int i = 0; i < n_apps; i++)
	{
		std::cout << "apps[" << i << "]:" << std::endl;
		apps[i]->prepareInput();
		for (int t = 0; t < n_targets; t++)
		{
			std::cout << " Target[" << t << "]: " << 
				apps[i]->getKernelFullName(targets[t]) << std::endl;
			
			apps[i]->setup();
			
			apps[i]->process(targets[t]);
			
			if (!apps[i]->verify())
				std::cout << "\tverify fail" << std::endl;
			else
				std::cout << "\tverify success" << std::endl;
		}
		apps[i]->release();
	}

	for (int i = 0; i < n_apps; i++)
		delete apps[i];
	
	std::cout << argv[0] << " done!!" << endl;
	//system("PAUSE");
	return 0;
}
