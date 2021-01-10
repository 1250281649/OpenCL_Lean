#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <OpenCLRuntime.h>

#define BUFSIZE     1024

void scalar_add(int n, const float* a, const float* b, float* result)
{
    int i = 0;
    for (i = 0; i < n; i++)
    {
        result[i] = a[i] + b[i];
    }

}
int main()
{
    OCL::OpenCLRuntime CLRT;

    float A[BUFSIZE];
    float B[BUFSIZE];
    float C[BUFSIZE];

    for (int i = 0; i < BUFSIZE; i++)
        A[i] = i;
    for (int i = 0; i < BUFSIZE; i++)
        B[i] = i + 1;
    std::vector<cl::Event> events;
    cl::Buffer bufferA = cl::Buffer(CLRT.context(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, BUFSIZE * sizeof(cl_float), (void*)&A[0]);
    cl::Buffer bufferB = cl::Buffer(CLRT.context(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, BUFSIZE * sizeof(cl_float), (void*)&B[0]);
    cl::Buffer bufferC = cl::Buffer(CLRT.context(), CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, BUFSIZE * sizeof(cl_float), (void*)&C[0]);

    std::set<std::string> buildOptions;
    cl::Kernel add_kernel = CLRT.buildKernel("E:\\VS2019\\OpenCL_Lean\\src\\add.cl", "vecadd", buildOptions);
    add_kernel.setArg<cl::Buffer>(0, bufferA);
    add_kernel.setArg<cl::Buffer>(1, bufferB);
    add_kernel.setArg<cl::Buffer>(2, bufferC);

    size_t gWI = BUFSIZE;
    CLRT.commandQueue().enqueueNDRangeKernel(add_kernel, cl::NullRange, cl::NDRange(BUFSIZE), cl::NullRange);

    float* output = (float*)CLRT.commandQueue().enqueueMapBuffer(bufferC, CL_TRUE, CL_MAP_READ, 0, BUFSIZE * sizeof(float));

    //release memory
    CLRT.commandQueue().enqueueUnmapMemObject(bufferC, (void*)output);

    return 0;
}
