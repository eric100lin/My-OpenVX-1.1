#include <iostream>
#include "vx.hpp"
#include "Application.hpp"
using namespace std;
using namespace cv;
using namespace OpenVX;

int main(int argc, char **argv)
{
	Context context;

	Application *apps[] = 
	{
		new AppOneIOneO(context, VX_KERNEL_NOT),
		new AppOneIOneO(context, VX_KERNEL_BOX_3x3),
		new AppOneIOneO(context, VX_KERNEL_GAUSSIAN_3x3),
		new AppTwoIOneO(context, VX_KERNEL_AND),
		new AppTwoIOneO(context, VX_KERNEL_OR),
		new AppTwoIOneO(context, VX_KERNEL_XOR),
		new AppTableLookup(context),
		//new AppHistogram(context),
	};
	int n_apps = sizeof(apps) / sizeof(apps[0]);
	for (int i = 0; i < n_apps; i++)
	{
		apps[i]->prepareInput();
		apps[i]->process();
		std::cout << "apps[" << i << "] verify ";
		if (!apps[i]->verify())
			std::cout << "fail!!" << endl;
		else
			std::cout << "successl!!" << endl;
		apps[i]->release();
	}

	std::cout << argv[0] << " done!!" << endl;
	system("PAUSE");
	return 0;
}
