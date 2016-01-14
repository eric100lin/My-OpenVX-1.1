#ifndef _NOT_NOT_GRAPH_H_
#define _NOT_NOT_GRAPH_H_

void not_not_cv(Mat inMat, Mat &outMat)
{
	bitwise_not(inMat,outMat);
	bitwise_not(outMat,outMat);
}

vx_status not_not_graph(vx_context context, Mat inMat, Mat &outMat)
{
	vx_status status = VX_SUCCESS;
	
	//Graph
	vx_graph graph = vxCreateGraph(context);
	CHECK_NOT_NULL(graph, "vxCreateGraph()");
	
	//Kernel
	vx_kernel not_kernel = vxGetKernelByName(context, VX_CL_NOT_NAME);
	CHECK_NOT_NULL(not_kernel, "OpenVX not kernel");
	
	//Node
	vx_node not_node = vxCreateGenericNode(graph, not_kernel);
	CHECK_NOT_NULL(not_node, "OpenVX not node");
	vx_node not_node_2 = vxCreateGenericNode(graph, not_kernel);
	CHECK_NOT_NULL(not_node_2, "OpenVX box3x3 node");
	
	//Data
	vx_image in = vxCreateImage(context, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8);
	CHECK_NOT_NULL(in, "input image");
	vx_image virtal_tmp = vxCreateVirtualImage(graph, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8);
	CHECK_NOT_NULL(virtal_tmp, "virtual tmp image");
	vx_image out = vxCreateImage(context, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8);
	CHECK_NOT_NULL(out, "output image");
	status = vxSetParameterByIndex(not_node, 0, (vx_reference)in);
	CHECK_STATUS(status, "set arg 0 of not_node");
	status = vxSetParameterByIndex(not_node, 1, (vx_reference)virtal_tmp);
	CHECK_STATUS(status, "set arg 1 of not_node");
	status = vxSetParameterByIndex(not_node_2, 0, (vx_reference)virtal_tmp);
	CHECK_STATUS(status, "set arg 0 of not_node_2");
	status = vxSetParameterByIndex(not_node_2, 1, (vx_reference)out);
	CHECK_STATUS(status, "set arg 1 of not_node_2");
	status = WriteImage(inMat, in);
	CHECK_STATUS(status, "WriteImage");
	
	//Verify and process
	status = vxVerifyGraph(graph);
	CHECK_STATUS(status, "vxVerifyGraph");
	status = vxProcessGraph(graph);
	CHECK_STATUS(status, "vxProcessGraph");
	
	//Read result back
	status = ReadImage(outMat, out);
	CHECK_STATUS(status, "ReadImage");
	
	//Qurey profiling information
	vx_perf_t perf_graph, perf_not, perf_not2;
	status = vxQueryGraph(graph, VX_GRAPH_ATTRIBUTE_PERFORMANCE, &perf_graph, sizeof(vx_perf_t));
	CHECK_STATUS(status, "vxQueryGraph");
	status = vxQueryNode(not_node, VX_NODE_ATTRIBUTE_PERFORMANCE, &perf_not, sizeof(vx_perf_t));
	CHECK_STATUS(status, "vxQueryNode(perf_not)");
	status = vxQueryNode(not_node_2, VX_NODE_ATTRIBUTE_PERFORMANCE, &perf_not2, sizeof(vx_perf_t));
	CHECK_STATUS(status, "vxQueryNode(perf_not2)");
	printf("=== Profiling information ===\n");
	printf("  function: %s\n", __FUNCTION__);
	printf("  graph avg=%llu ms\n", (unsigned long long int)perf_graph.avg/1000);
	printf("  not_node avg=%llu ms\n", (unsigned long long int)perf_not.avg/1000);
	printf("  not_node_2 avg=%llu ms\n", (unsigned long long int)perf_not2.avg/1000);
	printf("=============================\n");
	
	//Release
	status = vxReleaseImage(&in);
	CHECK_STATUS(status, "vxReleaseImage(&in)");
	status = vxReleaseImage(&virtal_tmp);
	CHECK_STATUS(status, "vxReleaseImage(&virtal_tmp)");
	status = vxReleaseImage(&out);
	CHECK_STATUS(status, "vxReleaseImage(&out)");
	status = vxReleaseNode(&not_node);
	CHECK_STATUS(status, "vxReleaseNode(&not_node)");
	status = vxReleaseNode(&not_node_2);
	CHECK_STATUS(status, "vxReleaseNode(&not_node_2)");
	status = vxReleaseKernel(&not_kernel);
	CHECK_STATUS(status, "vxReleaseKernel(&not_kernel)");
	status = vxReleaseGraph(&graph);
	CHECK_STATUS(status, "vxReleaseGraph");
	
	return status;
}

#endif