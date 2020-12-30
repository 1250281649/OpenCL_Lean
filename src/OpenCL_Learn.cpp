#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <OpenCLRuntime.h>

int main()
{
    OCL::OpenCLRuntime CLRT;
 /*   cl_platform_id* platform;
    cl_uint num_platform;
    cl_int err, plat_florm_index = -1;

    char* ext_data;
    size_t ext_size;
    const char icd_ext[] = "cl_khr_icd";

    err = clGetPlatformIDs(5, NULL, &num_platform);
    if (err < 0)
    {
        perror("Couldn't find any platforms\n");
        exit(-1);
    }

    printf("本机支持OpenCL的环境数量: %d\n", num_platform);

    platform = (cl_platform_id*)malloc(sizeof(cl_platform_id) * num_platform);
    clGetPlatformIDs(num_platform, platform, NULL);

    for (int i = 0; i < num_platform; i++)
    {
        err = clGetPlatformInfo(platform[i], CL_PLATFORM_EXTENSIONS, 0, NULL, &ext_size);
        if (err < 0)
        {
            perror("Couldn't read extension data\n");
            exit(1);
        }
        printf("缓存大小: %d\n", ext_size);

        ext_data = (char*)malloc(ext_size);
        clGetPlatformInfo(platform[i], CL_PLATFORM_EXTENSIONS, ext_size, ext_data, NULL);
        printf("平台%d支持的扩展功能:%s\n", i, ext_data);

        char* name = (char*)malloc(ext_size);
        clGetPlatformInfo(platform[i], CL_PLATFORM_NAME, ext_size, name, NULL);
        printf("平台%d是:%s\n", i, name);

        char* vendor = (char*)malloc(ext_size);
        clGetPlatformInfo(platform[i], CL_PLATFORM_VENDOR, ext_size, vendor, NULL);
        printf("平台%d的生产商是:%s\n", i, vendor);

        char* version = (char*)malloc(ext_size);
        clGetPlatformInfo(platform[i], CL_PLATFORM_VERSION, ext_size, version, NULL);
        printf("平台%d的版本是:%s\n", i, version);

        cl_uint numDevices = 0;
        cl_device_id* deviceIds;
        err = clGetDeviceIDs(platform[i], CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);
        if (err != CL_SUCCESS || numDevices <= 0)
        {
            perror("No GPU device found for platform\n");
            continue;
        }
        deviceIds = (cl_device_id*)malloc(sizeof(cl_device_id) * numDevices);
        err = clGetDeviceIDs(platform[i], CL_DEVICE_TYPE_GPU, numDevices, deviceIds, NULL);
        if (err != CL_SUCCESS)
        {
            perror("No GPU found for the platform.\n");
            continue;
        }

        cl_context_properties properties[] = {
            CL_CONTEXT_PLATFORM, (cl_context_properties)platform[i], 0
        };

        cl_context context = clCreateContext(properties, numDevices, deviceIds ,NULL, NULL, &err);
        if (context == NULL || err != CL_SUCCESS)
        {
            perror("Failed to create context.\n");
            continue;
        }

        cl_command_queue command_queue;
        command_queue = clCreateCommandQueue(context, deviceIds[0], CL_QUEUE_PROFILING_ENABLE, NULL);


        clReleaseCommandQueue(command_queue);
        free(ext_data);
        free(name);
        free(vendor);
        free(version);
    }

    free(platform);*/

    return 0;
}
