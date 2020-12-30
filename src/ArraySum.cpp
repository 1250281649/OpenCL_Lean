#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <Windows.h>
#include <CL/cl.h>

#define BUFSIZE     (pow(2, 15))

int convertToString(const char* fileName, std::string& s)
{
    size_t size;
    char* str;

    std::fstream f(fileName, (std::fstream::in | std::fstream::binary));

    if (f.is_open())
    {
        size_t fileSize;
        f.seekg(0, std::fstream::end);
        size = fileSize = (size_t)f.tellg();
        f.seekg(0, std::fstream::beg);

        str = new char[size + 1];
        if (!str)
        {
            f.close();
            return -1;
        }
        f.read(str, fileSize);
        f.close();
        str[size] = '\0';

        s = str;
        delete[] str;
        return 0;
    }
    printf("Failed to open file %s\n", fileName);
    return -1;
}

int main()
{
    std::cout << "test opencl for array sum" << std::endl;

    DWORD time_start, time_end;

    float* buf1 = NULL;
    float* buf2 = NULL;
    float* buf = NULL;

    buf1 = (float*)malloc(sizeof(float) * BUFSIZE);
    if (!buf1)
        return 0;
    buf2 = (float*)malloc(sizeof(float) * BUFSIZE);
    if (!buf2)
        return 0;
    buf = (float*)malloc(sizeof(float) * BUFSIZE);
    if (!buf)
        return 0;

    for (int i = 0; i < BUFSIZE; i++)
    {
        buf1[i] = (rand() % 32767);
        buf2[i] = (rand() % 32767);
    }

    time_start = GetTickCount64();
    //CPU computing
    for (int i = 0; i < BUFSIZE; i++)
    {
        buf[i] = buf1[i] + buf2[i];
    }
    time_end = GetTickCount64();
    std::cout << "CPU calculate time:" << (time_end - time_start) << "ms\n";

    //GPU computing
    cl_uint status;
    cl_platform_id* platform;
    cl_uint num_platform;
    status = clGetPlatformIDs(5, NULL, &num_platform);
    if (status != CL_SUCCESS)
    {
        perror("Couldn't find any platforms\n");
        return -1;
    }

    printf("本机支持OpenCL的环境数量: %d\n", num_platform);

    platform = (cl_platform_id*)malloc(sizeof(cl_platform_id) * num_platform);
    if (!platform) return -1;
    clGetPlatformIDs(num_platform, platform, NULL);

    cl_device_id device;
    status = clGetDeviceIDs(platform[0], CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    if (status != CL_SUCCESS)
    {
        printf("NO GPU device found!\n");
        return -1;
    }
    //创建context
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    //创建命令队列
    cl_command_queue command_queue = clCreateCommandQueue(context, device, 
        CL_QUEUE_PROFILING_ENABLE, NULL);
    //创建三个内存对象，并把buf1的内容通过隐式拷贝的方式拷贝到clBuf1, buf2通过显示
    //拷贝到clBuf2
    cl_mem clBuf1 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        BUFSIZE * sizeof(cl_float), buf1, NULL);
    cl_mem clBuf2 = clCreateBuffer(context, CL_MEM_READ_ONLY,
        BUFSIZE * sizeof(cl_float), NULL, NULL);
    cl_event writeEvt;
    status = clEnqueueWriteBuffer(command_queue, clBuf2, CL_TRUE, 0, BUFSIZE * sizeof(cl_float), buf2, 0, 0, &writeEvt);
    clWaitForEvents(1, &writeEvt);

    cl_mem clBuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, BUFSIZE * sizeof(cl_float), NULL, NULL);

    //kernel文件为add.cl
    const char* filename = "add.cl";
    std::string sourceStr;
    status = convertToString(filename, sourceStr);

    const char* source = sourceStr.c_str();
    size_t sourceSize[] = { strlen(source) };

    //创建程序对象
    cl_program program = clCreateProgramWithSource(context, 1, &source, sourceSize, NULL);
    //编译程序对象
    status = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (status != CL_SUCCESS)
    {
        printf("clBuild failed:%d\n", status);
        char tbuf[10240];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 10240, tbuf, NULL);
        printf("\n%s\n", tbuf);
        return -1;
    }

    //创建kernel对象
    cl_kernel kernel = clCreateKernel(program, "vecadd", NULL);
    //设置kernel参数
    cl_int clnum = BUFSIZE;
    clSetKernelArg(kernel, 0, sizeof(clnum), (void*)&clBuf1);
    clSetKernelArg(kernel, 1, sizeof(clnum), (void*)&clBuf2);
    clSetKernelArg(kernel, 2, sizeof(clnum), (void*)&clBuf);
    //执行kernel， range用1维， work items size为BUFSIZE
    cl_event ev;
    size_t global_work_size = BUFSIZE;

    time_start = GetTickCount64();
    clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_work_size, NULL, 0, NULL, &ev);
    clWaitForEvents(1, &ev);
    time_end = GetTickCount64();
    std::cout << "GPU calculate time:" << (time_end - time_start) << "ms\n";

    //数据拷回host内存
    cl_float* ptr = NULL;
    cl_event mapevt;
    ptr = (cl_float*)clEnqueueMapBuffer(command_queue, 
        clBuf,
        CL_TRUE, 
        CL_MAP_READ, 
        0, 
        BUFSIZE * sizeof(cl_float), 
        0, NULL, &mapevt, NULL);
    clWaitForEvents(1, &mapevt);

    //结果验证，和CPU计算的结果比较
    if (!memcmp(buf, ptr, BUFSIZE* sizeof(cl_float)))
        printf("Verify passed!\n");
    else    printf("Verify failed!\n");

    clReleaseMemObject(clBuf);
    clReleaseMemObject(clBuf1);
    clReleaseMemObject(clBuf2);
    clReleaseProgram(program);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);

    free(buf1);
    free(buf2);
    free(buf);
    return 0;
}