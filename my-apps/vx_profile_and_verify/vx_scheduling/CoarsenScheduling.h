#ifndef _COARSEN_SCHEDULING_HPP_
#define _COARSEN_SCHEDULING_HPP_
#include "vx.hpp"
#include "ProfileData.h"

namespace OpenVX
{
	void nodeCoarsen(ProfileData &profileData, int n_nodes, vx_kernel_e *kernel_es, Target *targets);
}

#endif