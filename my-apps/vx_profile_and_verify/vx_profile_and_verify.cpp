#include <iostream>
#include <VX/vx.h>
using namespace std;

int main(int argc, char **argv)
{
	vx_context context = vxCreateContext();
	CHECK_NOT_NULL(context, "vxCreateContext");
	cout << "Success create vx_context!!" << endl;
	
	
	
	status = vxReleaseContext(&context);
	CHECK_STATUS(status, "vxReleaseContext");
	
	cout << argv[0] << " done!!" << endl;
	return 0;
}
