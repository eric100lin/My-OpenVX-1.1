#define NOT_KERNEL_FILENAME "/home/thlin/openvx/experiment/My-OpenVX-1.0.1-fussion/kernels/opencl/vx_not.cl"
#define BOX3X3_KERNEL_FILENAME "/home/thlin/openvx/experiment/My-OpenVX-1.0.1-fussion/kernels/opencl/vx_box3x3.cl"
#define OPTIONS_STR "-I/home/thlin/openvx/experiment/My-OpenVX-1.0.1-fussion/kernels/opencl -I/home/thlin/openvx/experiment/My-OpenVX-1.0.1-fussion/include -DVX_CL_KERNEL"
#include <CL/cl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "my_cl_utils.h"
#include <sys/time.h>
#define SRC_IMG_NAME "lena.jpg"
#define IMG_WIDTH 640
#define IMG_HEIGHT 480

int invoke_ocl_kernel(IplImage *src_in, IplImage *out_img, const char *filename, const char *kernel_name, cl_device_type device_type)
{
	int i;
	cl_int status = 0;
    //struct timeval t1, t2;
	struct timespec t1, t2;
    double elapsedTime = 0.0;
	
	/*Step1: Getting platforms and choose an available one.*/
	cl_uint numPlatforms;	//the NO. of platforms
	cl_platform_id platform = NULL;	//the chosen platform
	status = clGetPlatformIDs(0, NULL, &numPlatforms);
	CHECK_OPENCL_ERROR(status, "Error: Getting platforms!");

	/*For clarity, choose the first available platform. */
	if(numPlatforms > 0)
	{
		cl_platform_id* platforms = (cl_platform_id* )malloc(numPlatforms* sizeof(cl_platform_id));
		status = clGetPlatformIDs(numPlatforms, platforms, NULL);
		CHECK_OPENCL_ERROR(status, "Error: Getting platforms!");
		platform = platforms[0];
		free(platforms);
	}
	
	/*Step 2:Query the platform and choose the first GPU device if has one.Otherwise use the CPU as device.*/
	cl_uint				numDevices = 0;
	cl_device_id        *devices = NULL;
	status = clGetDeviceIDs(platform, device_type, 0, NULL, &numDevices);	
	if (numDevices == 0)	//no GPU available.
	{
		printf("No GPU device available.\n");
		printf("Choose CPU as default device.\n");
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, NULL, &numDevices);
		CHECK_OPENCL_ERROR(status, "Error: Getting devices!");
		devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, numDevices, devices, NULL);
		CHECK_OPENCL_ERROR(status, "Error: Getting devices!");
	}
	else
	{
		devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, devices, NULL);
		CHECK_OPENCL_ERROR(status, "Error: Getting devices!");
	}
	/*char deviceName[1024];
	status = clGetDeviceInfo(devices[0], CL_DEVICE_NAME, sizeof(deviceName),
							 deviceName, NULL);
	CHECK_OPENCL_ERROR(status, "clGetDeviceInfo failed");
	printf("Device Name: %s; Device ID is %d\n", deviceName, devices[0]);*/
	
	/*Step 3: Create context.*/
	cl_context context = clCreateContext(NULL,1, devices, NULL, NULL, &status);
	CHECK_OPENCL_ERROR(status, "Error: Creating context");
	
	/*Step 4: Creating command queue associate with the context.*/
	cl_command_queue commandQueue = clCreateCommandQueue(context, devices[0], 0, &status);
	CHECK_OPENCL_ERROR(status, "Error: Creating command queue");
	
	/*Step 5: Create program object */
	char *sourceStrStr = NULL;
	status = read_file_string(filename, &sourceStrStr);
	if(status!=0 || sourceStrStr==NULL)
		return -1;
	//printf("sourceStrStr:\n%s\nOPTIONS_STR:\n%s\n\n", sourceStrStr, OPTIONS_STR);
	size_t sourceStrSize[] = {strlen(sourceStrStr)};
	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&sourceStrStr, sourceStrSize, &status);
	CHECK_OPENCL_ERROR(status, "clCreateProgramWithSource failed.");
	
	/*Step 6: Build program. */
	status=clBuildProgram(program, 1, devices, OPTIONS_STR, NULL, NULL);
    if(status != CL_SUCCESS)
    {
        if(status == CL_BUILD_PROGRAM_FAILURE)
        {
            cl_int logStatus;
            char *buildLog = NULL;
            size_t buildLogSize = 0;
            logStatus = clGetProgramBuildInfo (
                            program,
                            devices[0],
                            CL_PROGRAM_BUILD_LOG,
                            buildLogSize,
                            buildLog,
                            &buildLogSize);
            CHECK_OPENCL_ERROR(logStatus, "clGetProgramBuildInfo failed.");
            buildLog = (char*)malloc(buildLogSize);
            CHECK_ALLOCATION(buildLog, "Failed to allocate host memory. (buildLog)");
            memset(buildLog, 0, buildLogSize);
            logStatus = clGetProgramBuildInfo (
                            program,
                            devices[0],
                            CL_PROGRAM_BUILD_LOG,
                            buildLogSize,
                            buildLog,
                            NULL);
            if(logStatus != CL_SUCCESS)
            {
				printf("clGetProgramBuildInfo failed.\n");
                free(buildLog);
                return SDK_FAILURE;
            }
			printf(" ************************************************\n");
			printf("BUILD LOG:\n");
            printf(" ************************************************\n");
            printf("%s\n", buildLog);
            printf(" ************************************************\n");
            free(buildLog);
        }
        CHECK_OPENCL_ERROR(status, "clBuildProgram failed.");

		clReleaseCommandQueue(commandQueue);	//Release command queue.
		clReleaseContext(context);				//Release context.
		if (devices != NULL)
		{
			free(devices);
			devices = NULL;
		}
		free(sourceStrStr);
		sourceStrStr = NULL;
		return status;
	}
	
	/*Step X: Create kernel object */
	cl_kernel kernel = clCreateKernel(program, kernel_name, &status);
	CHECK_OPENCL_ERROR(status, "clCreateKernel failed.");
	
	for(i=0; i<10+1; i++)
	{
		IplImage *in_img = cvCloneImage(src_in);
		
		//gettimeofday(&t1, NULL);
		clock_gettime(CLOCK_MONOTONIC, &t1);
		
		/*Step 7: Initial input,output for the host and create memory objects for the kernel*/
		cl_mem inputBuffer = 
		clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, in_img->imageSize, (void *)in_img->imageData, &status);
		CHECK_OPENCL_ERROR(status, "clCreateBuffer for inputBuffer failed.");
		cl_mem outputBuffer = 
		clCreateBuffer(context, CL_MEM_WRITE_ONLY, out_img->imageSize, NULL, &status);
		CHECK_OPENCL_ERROR(status, "clCreateBuffer for outputBuffer failed.");

		/*Step 9: Sets Kernel arguments.*/
		unsigned int pidx = 0;
		int sx=1, sy=in_img->width;
		status = clSetKernelArg(kernel, pidx++, sizeof(cl_int), (void *)&in_img->width);
		CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
		status = clSetKernelArg(kernel, pidx++, sizeof(cl_int), (void *)&in_img->height);
		CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
		status = clSetKernelArg(kernel, pidx++, sizeof(cl_int), (void *)&sx);
		CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
		status = clSetKernelArg(kernel, pidx++, sizeof(cl_int), (void *)&sy);
		CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
		status = clSetKernelArg(kernel, pidx++, sizeof(cl_mem), (void *)&inputBuffer);
		CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
		status = clSetKernelArg(kernel, pidx++, sizeof(cl_int), (void *)&out_img->width);
		CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
		status = clSetKernelArg(kernel, pidx++, sizeof(cl_int), (void *)&out_img->height);
		CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
		status = clSetKernelArg(kernel, pidx++, sizeof(cl_int), (void *)&sx);
		CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
		status = clSetKernelArg(kernel, pidx++, sizeof(cl_int), (void *)&sy);
		CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
		status = clSetKernelArg(kernel, pidx++, sizeof(cl_mem), (void *)&outputBuffer);
		CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
	
		/*Step 10: Running the kernel.*/
		size_t global_work_sizes[2] = { in_img->width, in_img->height };
		status = clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, global_work_sizes, NULL, 0, NULL, NULL);
		CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");
	
		/*Step 11: Read the cout put back to host memory.*/
		status = clEnqueueReadBuffer(commandQueue, outputBuffer, CL_TRUE, 0, 
									 out_img->imageSize, out_img->imageData, 0, NULL, NULL);
		CHECK_OPENCL_ERROR(status, "clEnqueueReadBuffer failed.");
		
		status = clReleaseMemObject(inputBuffer);		//Release mem object.
		CHECK_OPENCL_ERROR(status, "clReleaseMemObject inputBuffer failed.");
		status = clReleaseMemObject(outputBuffer);
		CHECK_OPENCL_ERROR(status, "clReleaseMemObject outputBuffer failed.");
		
		//gettimeofday(&t2, NULL);
		clock_gettime(CLOCK_MONOTONIC, &t2);
		if(i!=0)
		{
			//elapsedTime += (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
			//elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
			elapsedTime += (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
			elapsedTime += (t2.tv_nsec - t1.tv_nsec) / 1000000.0;   // ns to ms
		}
		cvReleaseImage(&in_img);
	}
	
	printf("avg. elapsedTime: %lf ms\n", elapsedTime/10);
	
	/*Step 12: Clean the resources.*/
	status = clReleaseKernel(kernel);				//Release kernel.
	CHECK_OPENCL_ERROR(status, "clReleaseKernel failed.");
	status = clReleaseProgram(program);				//Release the program object.
	CHECK_OPENCL_ERROR(status, "clReleaseProgram failed.");
	status = clReleaseCommandQueue(commandQueue);	//Release  Command queue.
	CHECK_OPENCL_ERROR(status, "clReleaseCommandQueue failed.");
	status = clReleaseContext(context);				//Release context.
	CHECK_OPENCL_ERROR(status, "clReleaseContext failed.");
	if (devices != NULL)
	{
		free(devices);
		devices = NULL;
	}
	free(sourceStrStr);
	sourceStrStr = NULL;
	
	return status;
}

int main(int argc, char **argv)
{
	int i;
	IplImage *src = cvLoadImage(SRC_IMG_NAME, CV_LOAD_IMAGE_COLOR);
	CvSize dst_cvsize;
	dst_cvsize.width = IMG_WIDTH;
	dst_cvsize.height = IMG_HEIGHT;
	IplImage *src_resized = cvCreateImage(dst_cvsize, src->depth, src->nChannels);
	IplImage *src_gray = cvCreateImage(dst_cvsize, IPL_DEPTH_8U, 1);
	cvResize(src, src_resized, CV_INTER_NN);
	cvCvtColor(src_resized, src_gray, CV_RGB2GRAY);
	
	printf("CL_DEVICE_TYPE_CPU:\n");
	for(i=0;i<10;i++)
	{
		IplImage *out_img1 = cvCreateImage(dst_cvsize, IPL_DEPTH_8U, 1);
		IplImage *out_img2 = cvCreateImage(dst_cvsize, IPL_DEPTH_8U, 1);
		cl_int status = invoke_ocl_kernel(src_gray,out_img1,BOX3X3_KERNEL_FILENAME, "vx_box3x3", CL_DEVICE_TYPE_CPU);
		if(status==CL_SUCCESS)
		{
			status = invoke_ocl_kernel(out_img1,out_img2,NOT_KERNEL_FILENAME, "vx_not", CL_DEVICE_TYPE_CPU);
		}
		cvReleaseImage(&out_img1);
		cvReleaseImage(&out_img2);
	}
	
	printf("\nCL_DEVICE_TYPE_GPU:\n");
	for(i=0;i<10;i++)
	{
		IplImage *out_img1 = cvCreateImage(dst_cvsize, IPL_DEPTH_8U, 1);
		IplImage *out_img2 = cvCreateImage(dst_cvsize, IPL_DEPTH_8U, 1);
		cl_int status = invoke_ocl_kernel(src_gray,out_img1,BOX3X3_KERNEL_FILENAME, "vx_box3x3", CL_DEVICE_TYPE_GPU);
		if(status==CL_SUCCESS)
		{
			status = invoke_ocl_kernel(out_img1,out_img2,NOT_KERNEL_FILENAME, "vx_not", CL_DEVICE_TYPE_GPU);
		}
		cvReleaseImage(&out_img1);
		cvReleaseImage(&out_img2);
	}
	
	cvReleaseImage(&src);
	cvReleaseImage(&src_resized);
	cvReleaseImage(&src_gray);
	return 0;
}
