#include "ProfileData.h"
#include <stdio.h>
#include <string.h>
#define PROFILE_FILE_NAME "profiling.csv"
using namespace std;
using namespace OpenVX;

ProfileData::ProfileData()
{
	string line;
	fstream profile(PROFILE_FILE_NAME, std::fstream::in);
	while (std::getline(profile, line))
	{
		size_t comma1 = line.find(",");
		size_t comma2 = line.rfind(",");
		string k_name = line.substr(0, comma1);
		string compute_str = line.substr(comma1 + 1, comma2 - comma1 - 1);
		string transfer_str = line.substr(comma2 + 1, line.length() - comma1 - 1);

		size_t colon = k_name.find(":");
		string k_name_target = k_name.substr(0, colon);
		string k_name_kernel = k_name.substr(colon+1, k_name.length()- colon - 1);

		Target t_enum = Kernel::getTargetEnumFromName(k_name_target.c_str());
		int target_index = (int) t_enum;
		vx_kernel_e kernel_e = Kernel::getKernelEnumFromName(k_name_kernel);

		Triple_t transfer = transfers[kernel_e];
		transfer.time[target_index] = (float)atof(transfer_str.c_str());
		transfers[kernel_e] = transfer;

		Triple_t compute = computes[kernel_e];
		compute.time[target_index] = (float)atof(compute_str.c_str());
		computes[kernel_e] = compute;
	}
	profile.close();
}

float ProfileData::getImproveFator(vx_kernel_e kernel_e)
{
	Triple_t compute_of_kernel_e = computes[kernel_e];
	float min_compute = compute_of_kernel_e.time[0];
	float max_compute = compute_of_kernel_e.time[0];
	for (int i = 1; i < N_TARGETS; i++)
	{
		if (compute_of_kernel_e.time[i] > max_compute)
			max_compute = compute_of_kernel_e.time[i];
		if (compute_of_kernel_e.time[i] < min_compute)
			min_compute = compute_of_kernel_e.time[i];
	}
	return max_compute / min_compute;
}

static struct { enum vx_kernel_e value; int n_parameter; } parameter_tables[] = {
	{ VX_KERNEL_NOT, 2 },
	{ VX_KERNEL_BOX_3x3, 2 },
	{ VX_KERNEL_GAUSSIAN_3x3, 2 },
	{ VX_KERNEL_AND, 3 },
	{ VX_KERNEL_XOR, 3 },
	{ VX_KERNEL_ADD, 3 },
	{ VX_KERNEL_SUBTRACT, 3 },
	{ VX_KERNEL_THRESHOLD, 2 },
	{ VX_KERNEL_MAX_1_0, -1 }
};

int look_for_parameter(enum vx_kernel_e kernel_e)
{
	for (int i = 0; parameter_tables[i].value != VX_KERNEL_MAX_1_0; i++)
	{
		if (parameter_tables[i].value == kernel_e)
			return parameter_tables[i].n_parameter;
	}
	return -1;
}

float ProfileData::getTransferTime(vx_kernel_e kernel_e, Target target_e)
{
	int n_parameter = look_for_parameter(kernel_e);
	return (transfers[kernel_e].time[target_e] / n_parameter);
}

float ProfileData::getComputationTime(vx_kernel_e kernel_e, Target target)
{
	return computes[kernel_e].time[target];
}

static Target pTargets[N_TARGETS] = { TARGET_C_MODEL , TARGET_OPENCL , TARGET_HEXAGON };

Target ProfileData::getMiniComputeTimeTarget(vx_kernel_e kernel_e)
{
	Triple_t compute = computes[kernel_e];
	float minimal = compute.time[0];
	Target miniComputeTimeTarget = pTargets[0];
	for (int i = 1; i < N_TARGETS; i++)
	{
		if (compute.time[i] < minimal)
		{
			minimal = compute.time[i];
			miniComputeTimeTarget = pTargets[i];
		}
	}
	return miniComputeTimeTarget;
}

Target ProfileData::getMiniTurnAroundTimeTarget(vx_kernel_e kernel_e)
{
	Triple_t compute = computes[kernel_e];
	Triple_t transfer = transfers[kernel_e];
	float minimal = compute.time[0] + transfer.time[0];
	Target miniTurnAroundTimeTarget = pTargets[0];
	for (int i = 1; i < N_TARGETS; i++)
	{
		if (compute.time[i] + transfer.time[i] < minimal)
		{
			minimal = compute.time[i] + transfer.time[i];
			miniTurnAroundTimeTarget = pTargets[i];
		}
	}
	return miniTurnAroundTimeTarget;
}
