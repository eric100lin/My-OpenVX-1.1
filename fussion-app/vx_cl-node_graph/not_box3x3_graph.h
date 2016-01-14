#ifndef _NOT_BOX3X3_GRAPH_H_
#define _NOT_BOX3X3_GRAPH_H_

void my_box3x3(IplImage *in_img, IplImage *result_img)
{
	int x, y;
	int width = in_img->width;
	int height = in_img->height;
	int sx = 1;
	int sy = width;
	
	#define vxImagePixel(ptr, x, y, sx, sy) \
		(((unsigned char *)ptr)[((y) * sy) + ((x) * sx)])
	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			int16_t sum = 0;
			if (y == 0 || x == 0 || x == (width - 1) || y == (height - 1))
			{
				vxImagePixel(result_img->imageData, x, y, sx, sy) = 
					vxImagePixel(in_img->imageData, x, y, sx, sy);
				continue; // border mode...
			}
			sum += vxImagePixel(in_img->imageData, x-1, y-1, sx, sy);
			sum += vxImagePixel(in_img->imageData, x+0, y-1, sx, sy);
			sum += vxImagePixel(in_img->imageData, x+1, y-1, sx, sy);
			sum += vxImagePixel(in_img->imageData, x-1, y+0, sx, sy);
			sum += vxImagePixel(in_img->imageData, x+0, y+0, sx, sy);
			sum += vxImagePixel(in_img->imageData, x+1, y+0, sx, sy);
			sum += vxImagePixel(in_img->imageData, x-1, y+1, sx, sy);
			sum += vxImagePixel(in_img->imageData, x+0, y+1, sx, sy);
			sum += vxImagePixel(in_img->imageData, x+1, y+1, sx, sy);
			
			sum /= 9;
			vxImagePixel(result_img->imageData, x, y, sx, sy) = (unsigned char)sum;
		}
	}
}

void not_box3x3_cv(Mat inMat, Mat &outMat)
{
	bitwise_not(inMat,inMat);
	Mat tmp = inMat.clone();
	//blur(tmp, outMat, Size(3,3));
	IplImage tmp_Ipl(tmp), out_Ipl(outMat);
	my_box3x3(&tmp_Ipl, &out_Ipl);
}

vx_status not_box3x3_graph(vx_context context, Mat inMat, Mat &outMat)
{
	vx_status status = VX_SUCCESS;
	
	//Graph
	vx_graph graph = vxCreateGraph(context);
	CHECK_NOT_NULL(graph, "vxCreateGraph()");
	
	//Kernel
	vx_kernel not_kernel = vxGetKernelByName(context, VX_CL_NOT_NAME);
	CHECK_NOT_NULL(not_kernel, "OpenVX not kernel");
	vx_kernel box3x3_kernel = vxGetKernelByName(context, VX_CL_BOX3X3_NAME);
	CHECK_NOT_NULL(box3x3_kernel, "OpenVX box3x3 kernel");
	
	//Node
	vx_node not_node = vxCreateGenericNode(graph, not_kernel);
	CHECK_NOT_NULL(not_node, "OpenVX not node");
	vx_node box3x3_node = vxCreateGenericNode(graph, box3x3_kernel);
	CHECK_NOT_NULL(box3x3_node, "OpenVX box3x3 node");
	
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
	status = vxSetParameterByIndex(box3x3_node, 0, (vx_reference)virtal_tmp);
	CHECK_STATUS(status, "set arg 0 of box3x3_node");
	status = vxSetParameterByIndex(box3x3_node, 1, (vx_reference)out);
	CHECK_STATUS(status, "set arg 1 of box3x3_node");
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
	vx_perf_t perf_graph, perf_not, perf_box3x3;
	status = vxQueryGraph(graph, VX_GRAPH_ATTRIBUTE_PERFORMANCE, &perf_graph, sizeof(vx_perf_t));
	CHECK_STATUS(status, "vxQueryGraph");
	status = vxQueryNode(not_node, VX_NODE_ATTRIBUTE_PERFORMANCE, &perf_not, sizeof(vx_perf_t));
	CHECK_STATUS(status, "vxQueryNode(perf_not)");
	status = vxQueryNode(box3x3_node, VX_NODE_ATTRIBUTE_PERFORMANCE, &perf_box3x3, sizeof(vx_perf_t));
	CHECK_STATUS(status, "vxQueryNode(perf_box3x3)");
	printf("=== Profiling information ===\n");
	printf("  function: %s\n", __FUNCTION__);
	printf("  graph avg=%llu ms\n", (unsigned long long int)perf_graph.avg/1000);
	printf("  not_node avg=%llu ms\n", (unsigned long long int)perf_not.avg/1000);
	printf("  box3x3_node avg=%llu ms\n", (unsigned long long int)perf_box3x3.avg/1000);
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
	status = vxReleaseNode(&box3x3_node);
	CHECK_STATUS(status, "vxReleaseNode(&box3x3_node)");
	status = vxReleaseKernel(&not_kernel);
	CHECK_STATUS(status, "vxReleaseKernel(&not_kernel)");
	status = vxReleaseKernel(&box3x3_kernel);
	CHECK_STATUS(status, "vxReleaseKernel(&box3x3_kernel)");
	status = vxReleaseGraph(&graph);
	CHECK_STATUS(status, "vxReleaseGraph");
	
	return status;
}

#endif