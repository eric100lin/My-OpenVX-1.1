#ifndef MY_CL_UTILS_H_
#define MY_CL_UTILS_H_
#include <CL/opencl.h>
#define SDK_FAILURE 1

const char *getOpenCLErrorCodeStr(int input)
{
    int errorCode = input;
    switch(errorCode)
    {
		case CL_DEVICE_NOT_FOUND:
			return "CL_DEVICE_NOT_FOUND";
		case CL_DEVICE_NOT_AVAILABLE:
			return "CL_DEVICE_NOT_AVAILABLE";
		case CL_COMPILER_NOT_AVAILABLE:
			return "CL_COMPILER_NOT_AVAILABLE";
		case CL_MEM_OBJECT_ALLOCATION_FAILURE:
			return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
		case CL_OUT_OF_RESOURCES:
			return "CL_OUT_OF_RESOURCES";
		case CL_OUT_OF_HOST_MEMORY:
			return "CL_OUT_OF_HOST_MEMORY";
		case CL_PROFILING_INFO_NOT_AVAILABLE:
			return "CL_PROFILING_INFO_NOT_AVAILABLE";
		case CL_MEM_COPY_OVERLAP:
			return "CL_MEM_COPY_OVERLAP";
		case CL_IMAGE_FORMAT_MISMATCH:
			return "CL_IMAGE_FORMAT_MISMATCH";
		case CL_IMAGE_FORMAT_NOT_SUPPORTED:
			return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
		case CL_BUILD_PROGRAM_FAILURE:
			return "CL_BUILD_PROGRAM_FAILURE";
		case CL_MAP_FAILURE:
			return "CL_MAP_FAILURE";
		case CL_MISALIGNED_SUB_BUFFER_OFFSET:
			return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
		case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
			return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
		case CL_INVALID_VALUE:
			return "CL_INVALID_VALUE";
		case CL_INVALID_DEVICE_TYPE:
			return "CL_INVALID_DEVICE_TYPE";
		case CL_INVALID_PLATFORM:
			return "CL_INVALID_PLATFORM";
		case CL_INVALID_DEVICE:
			return "CL_INVALID_DEVICE";
		case CL_INVALID_CONTEXT:
			return "CL_INVALID_CONTEXT";
		case CL_INVALID_QUEUE_PROPERTIES:
			return "CL_INVALID_QUEUE_PROPERTIES";
		case CL_INVALID_COMMAND_QUEUE:
			return "CL_INVALID_COMMAND_QUEUE";
		case CL_INVALID_HOST_PTR:
			return "CL_INVALID_HOST_PTR";
		case CL_INVALID_MEM_OBJECT:
			return "CL_INVALID_MEM_OBJECT";
		case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
			return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
		case CL_INVALID_IMAGE_SIZE:
			return "CL_INVALID_IMAGE_SIZE";
		case CL_INVALID_SAMPLER:
			return "CL_INVALID_SAMPLER";
		case CL_INVALID_BINARY:
			return "CL_INVALID_BINARY";
		case CL_INVALID_BUILD_OPTIONS:
			return "CL_INVALID_BUILD_OPTIONS";
		case CL_INVALID_PROGRAM:
			return "CL_INVALID_PROGRAM";
		case CL_INVALID_PROGRAM_EXECUTABLE:
			return "CL_INVALID_PROGRAM_EXECUTABLE";
		case CL_INVALID_KERNEL_NAME:
			return "CL_INVALID_KERNEL_NAME";
		case CL_INVALID_KERNEL_DEFINITION:
			return "CL_INVALID_KERNEL_DEFINITION";
		case CL_INVALID_KERNEL:
			return "CL_INVALID_KERNEL";
		case CL_INVALID_ARG_INDEX:
			return "CL_INVALID_ARG_INDEX";
		case CL_INVALID_ARG_VALUE:
			return "CL_INVALID_ARG_VALUE";
		case CL_INVALID_ARG_SIZE:
			return "CL_INVALID_ARG_SIZE";
		case CL_INVALID_KERNEL_ARGS:
			return "CL_INVALID_KERNEL_ARGS";
		case CL_INVALID_WORK_DIMENSION:
			return "CL_INVALID_WORK_DIMENSION";
		case CL_INVALID_WORK_GROUP_SIZE:
			return "CL_INVALID_WORK_GROUP_SIZE";
		case CL_INVALID_WORK_ITEM_SIZE:
			return "CL_INVALID_WORK_ITEM_SIZE";
		case CL_INVALID_GLOBAL_OFFSET:
			return "CL_INVALID_GLOBAL_OFFSET";
		case CL_INVALID_EVENT_WAIT_LIST:
			return "CL_INVALID_EVENT_WAIT_LIST";
		case CL_INVALID_EVENT:
			return "CL_INVALID_EVENT";
		case CL_INVALID_OPERATION:
			return "CL_INVALID_OPERATION";
		case CL_INVALID_GL_OBJECT:
			return "CL_INVALID_GL_OBJECT";
		case CL_INVALID_BUFFER_SIZE:
			return "CL_INVALID_BUFFER_SIZE";
		case CL_INVALID_MIP_LEVEL:
			return "CL_INVALID_MIP_LEVEL";
		case CL_INVALID_GLOBAL_WORK_SIZE:
			return "CL_INVALID_GLOBAL_WORK_SIZE";
		case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR:
			return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
		case CL_PLATFORM_NOT_FOUND_KHR:
			return "CL_PLATFORM_NOT_FOUND_KHR";
			//case CL_INVALID_PROPERTY_EXT:
			//    return "CL_INVALID_PROPERTY_EXT";
		case CL_DEVICE_PARTITION_FAILED_EXT:
			return "CL_DEVICE_PARTITION_FAILED_EXT";
		case CL_INVALID_PARTITION_COUNT_EXT:
			return "CL_INVALID_PARTITION_COUNT_EXT";
		case CL_INVALID_DEVICE_QUEUE:
			return "CL_INVALID_DEVICE_QUEUE";
		case CL_INVALID_PIPE_SIZE:
			return "CL_INVALID_PIPE_SIZE";

		default:
			return "unknown error code";
    }
}

#define CHECK_ALLOCATION(actual, msg) \
    if(actual == NULL) \
    { \
        error(msg); \
        printf("Location: %s:%d\n", __FILE__, __LINE__); \
        return SDK_FAILURE; \
    }

#define CHECK_OPENCL_ERROR(actual, msg) \
    if(actual!=CL_SUCCESS) \
    { \
		printf("Error: %s Error code : %s\n", msg, getOpenCLErrorCodeStr(actual)); \
		printf("Location: %s:%d\n", __FILE__, __LINE__); \
        return SDK_FAILURE; \
    }

/* convert the kernel file into a string */
int read_file_string(const char *filename, char **file_str)
{
	size_t fileSize;
	FILE *pFile = fopen(filename,"r");
	
	if (pFile!=NULL)
	{
		fseek(pFile, 0, SEEK_END);
		fileSize = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);
		*file_str = (char *)malloc(sizeof(char)*(fileSize+1));
		if(*file_str == NULL)
		{
			fclose(pFile);
			return -1;
		}

		fread(*file_str, fileSize, 1, pFile);
		fclose(pFile);
		(*file_str)[fileSize] = '\0';
		return 0;
	}
	printf("Error: failed to open file:\n%s\n", filename);
	return -1;
}
	
#endif