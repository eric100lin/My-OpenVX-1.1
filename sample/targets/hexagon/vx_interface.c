/*
 * Copyright (c) 2016 National Tsing Hua University.
 * \brief The OpenCL OpenVX Kernel Interfaces
 * \author Tzu-Hsiang Lin <thlin@pllab.cs.nthu.edu.tw>
 */

#include <VX/vx.h>
#include <VX/vx_helper.h>
#include <VX/vx_khr_variants.h>
#include <vx_debug.h>

#include <vx_internal.h>
#include <vx_interface.h>
#include <hexagon_model.h>

static const vx_char name[VX_MAX_TARGET_NAME] = "android.hexagon";

static vx_kernel_description_t *target_kernels[] = {
    &not_kernel,
    &box3x3_kernel,
    &gaussian3x3_kernel,
    &and_kernel,
    &xor_kernel,
};
static vx_uint32 num_target_kernels = dimof(target_kernels);

vx_status vxTargetInit(vx_target target)
{
	vx_status status = VX_FAILURE;
    if (target)
    {
        strncpy(target->name, name, VX_MAX_TARGET_NAME);
        target->priority = VX_TARGET_PRIORITY_HEXAGON;
		
		status = vxHexagonInit();
		if(status != VX_SUCCESS)
			return status;
		
		status = vxInitializeTarget(target, target_kernels, num_target_kernels);
    }
	return status;
}

vx_status vxTargetDeinit(vx_target target)
{
	vx_status status_hex = vxHexagonDeInit();
	vx_status status_target = vxDeinitializeTarget(target);
	
	if(status_hex != VX_SUCCESS)
	{
		VX_PRINT(VX_ZONE_API,"vxTargetDeinit returned %d from vxHexagonDeInit()\n", status_hex);
		return status_hex;
	}
	
	VX_PRINT(VX_ZONE_API,"vxTargetDeinit returned %d from vxDeinitializeTarget()\n", status_target);
	return status_target;
}

vx_status vxTargetSupports(vx_target target,
                           vx_char targetName[VX_MAX_TARGET_NAME],
                           vx_char kernelName[VX_MAX_KERNEL_NAME],
#if defined(EXPERIMENTAL_USE_VARIANTS)
                           vx_char variantName[VX_MAX_VARIANT_NAME],
#endif
                           vx_uint32 *pIndex)
{
	vx_status status = VX_ERROR_NOT_SUPPORTED;
    if (strncmp(targetName, name, VX_MAX_TARGET_NAME) == 0)
    {
        vx_uint32 k = 0u;
        for (k = 0u; k < target->num_kernels; k++)
        {
            if (strncmp(kernelName, target->kernels[k].name, VX_MAX_KERNEL_NAME) == 0)
            {
                status = VX_SUCCESS;
                if (pIndex) *pIndex = k;
                break;
            }
        }
    }
	return status;	
}

vx_action vxTargetProcess(vx_target target, vx_node_t *nodes[], vx_size startIndex, vx_size numNodes)
{
    vx_action action = VX_ACTION_CONTINUE;
    vx_status status = VX_SUCCESS;
    vx_size n = 0;
    for (n = startIndex; (n < (startIndex + numNodes)) && (action == VX_ACTION_CONTINUE); n++)
    {
        VX_PRINT(VX_ZONE_GRAPH,"Executing Kernel %s:%d in Nodes[%u] on target %s\n",
            nodes[n]->kernel->name,
            nodes[n]->kernel->enumeration,
            n,
            nodes[n]->base.context->targets[nodes[n]->affinity].name);

        vxStartCapture(&nodes[n]->perf);
        status = nodes[n]->kernel->function((vx_node)nodes[n],
                                            (vx_reference *)nodes[n]->parameters,
                                            nodes[n]->kernel->signature.num_parameters);
        nodes[n]->executed = vx_true_e;
        nodes[n]->status = status;
        vxStopCapture(&nodes[n]->perf);

        VX_PRINT(VX_ZONE_GRAPH,"kernel %s returned %d\n", nodes[n]->kernel->name, status);

        if (status == VX_SUCCESS)
        {
            /* call the callback if it is attached */
            if (nodes[n]->callback)
            {
                action = nodes[n]->callback((vx_node)nodes[n]);
                VX_PRINT(VX_ZONE_GRAPH,"callback returned action %d\n", action);
            }
        }
        else
        {
            action = VX_ACTION_ABANDON;
            VX_PRINT(VX_ZONE_ERROR, "Abandoning Graph due to error (%d)!\n", status);
        }
    }
    return action;
}

vx_status vxTargetVerify(vx_target target, vx_node_t *node)
{
    vx_status status = VX_SUCCESS;
    return status;
}

vx_kernel vxTargetAddKernel(vx_target target,
                            vx_char name[VX_MAX_KERNEL_NAME],
                            vx_enum enumeration,
                            vx_kernel_f func_ptr,
                            vx_uint32 numParams,
                            vx_kernel_validate_f validate,
                            vx_kernel_input_validate_f input,
                            vx_kernel_output_validate_f output,
                            vx_kernel_initialize_f initialize,
                            vx_kernel_deinitialize_f deinitialize)
{
    vx_uint32 k = 0u;
    vx_kernel_t *kernel = NULL;
    // vxSemWait(&target->base.lock);
    for (k = 0; k < VX_INT_MAX_KERNELS; k++)
    {
        kernel = &(target->kernels[k]);
        if ((kernel->enabled == vx_false_e) &&
            (kernel->enumeration == VX_KERNEL_INVALID))
        {
            vxInitializeKernel(target->base.context,
                               kernel,
                               enumeration, func_ptr, name,
                               NULL, numParams,
                               validate, input, output, initialize, deinitialize);
            VX_PRINT(VX_ZONE_KERNEL, "Reserving %s Kernel[%u] for %s\n", target->name, k, kernel->name);
            target->num_kernels++;
            break;
        }
        kernel = NULL;
    }
    return (vx_kernel)kernel;
}

