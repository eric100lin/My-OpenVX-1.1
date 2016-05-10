#ifndef _NOT_GRAPH_H_
#define _NOT_GRAPH_H_

void not_cv(Mat inMat, Mat &outMat)
{
	bitwise_not(inMat, outMat);
}

vx_status not_graph(vx_context context, Mat inMat, Mat &outMat)
{
	int i;
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
	
	//Data
	vx_image in = vxCreateImage(context, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8);
	CHECK_NOT_NULL(in, "input image");
	vx_image out = vxCreateImage(context, IMG_WIDTH, IMG_HEIGHT, VX_DF_IMAGE_U8);
	CHECK_NOT_NULL(out, "output image");
	status = vxSetParameterByIndex(not_node, 0, (vx_reference)in);
	CHECK_STATUS(status, "set arg 0 of not_node");
	status = vxSetParameterByIndex(not_node, 1, (vx_reference)out);
	CHECK_STATUS(status, "set arg 1 of not_node");
	
	//Verify graph once
	status = vxVerifyGraph(graph);
	CHECK_STATUS(status, "vxVerifyGraph");
	
	//Profiling information
	vx_uint64 elapsedTime_graph = 0;
	vx_uint64 elapsedTime_not_node = 0;
	vx_perf_t perf_graph, perf_not;
	
	//Process graph ten times
	for(i=0; i</*10+1*/1; i++)
	{
		//Write input image into vx_image
		status = WriteImage(inMat, in);
		CHECK_STATUS(status, "WriteImage");
		
		//Processing
		status = vxProcessGraph(graph);
		CHECK_STATUS(status, "vxProcessGraph");
	
		//Read result back
		status = ReadImage(outMat, out);
		CHECK_STATUS(status, "ReadImage");
		
		//Qurey profiling information
		//if(i!=0)
		{
			status = vxQueryGraph(graph, VX_GRAPH_PERFORMANCE, &perf_graph, sizeof(vx_perf_t));
			CHECK_STATUS(status, "vxQueryGraph");
			status = vxQueryNode(not_node, VX_NODE_PERFORMANCE, &perf_not, sizeof(vx_perf_t));
			CHECK_STATUS(status, "vxQueryNode(perf_not)");
			elapsedTime_graph += perf_graph.tmp;
			elapsedTime_not_node += perf_not.tmp;
		}
	}
	
	printf("=== Profiling information ===\n");
	printf("  function: %s\n", __FUNCTION__);
	printf("  graph avg=%llu us\n", (unsigned long long int)elapsedTime_graph/(1000*10));
	printf("  not_node avg=%llu us\n", (unsigned long long int)elapsedTime_not_node/(1000*10));
	printf("=============================\n");
	
	//Release
	status = vxReleaseImage(&in);
	CHECK_STATUS(status, "vxReleaseImage(&in)");
	status = vxReleaseImage(&out);
	CHECK_STATUS(status, "vxReleaseImage(&out)");
	status = vxReleaseNode(&not_node);
	CHECK_STATUS(status, "vxReleaseNode(&not_node)");
	status = vxReleaseKernel(&not_kernel);
	CHECK_STATUS(status, "vxReleaseKernel(&not_kernel)");
	status = vxReleaseGraph(&graph);
	CHECK_STATUS(status, "vxReleaseGraph");
	
	return status;
}

#endif