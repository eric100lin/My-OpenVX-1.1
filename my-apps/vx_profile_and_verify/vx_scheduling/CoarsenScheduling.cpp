#include "CoarsenScheduling.h"
#include "ProfileData.h"
using namespace OpenVX;

void OpenVX::nodeCoarsen(ProfileData &profileData, int n_nodes, vx_kernel_e *kernel_es, Target *targets)
{
	for (int i = 0; i < n_nodes; i++)
	{
		targets[i] = profileData.getMiniComputeTimeTarget(kernel_es[i]);
	}
}