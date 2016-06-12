#ifndef _PROFILE_DATA_H_
#define _PROFILE_DATA_H_
#include "vx.hpp"
#include <map>
#define N_TARGETS 3

namespace OpenVX
{
	typedef struct Triple
	{
		float time[N_TARGETS];
	} Triple_t;

	class ProfileData
	{
	private:
		std::map<vx_kernel_e, Triple_t> transfers;
		std::map<vx_kernel_e, Triple_t> computes;
	public:
		ProfileData();
		float getImproveFator(vx_kernel_e kernel_e);

		float getTransferTime(vx_kernel_e kernel_e, Target target_e);
		float getComputationTime(vx_kernel_e kernel_e, Target target);
		Target getMiniComputeTimeTarget(vx_kernel_e kernel_e);
		Target getMiniTurnAroundTimeTarget(vx_kernel_e kernel_e);
	};
}

#endif