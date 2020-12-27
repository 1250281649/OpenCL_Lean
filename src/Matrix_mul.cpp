#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <fstream>
#include <CL/cl.h>

//A=M*N
//B=N*K
//C=M*K
#define MATRIX_M	2	//A height
#define MATRIX_N	3	//A width, B height
#define MATRIX_K	2	//B width

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
	printf("test matrix mulitiply");
	float A[MATRIX_M][MATRIX_N];
	float B[MATRIX_N][MATRIX_K];
	float C[MATRIX_M][MATRIX_K];

	for (int i = 0; i < MATRIX_M; i++)
		for(int j=0; j< MATRIX_N; j++)
			A[i][j] = i* MATRIX_N+j;
	for (int i = 0; i < MATRIX_N; i++)
		for(int j=0; j< MATRIX_K; j++)
			B[i][j] = i* MATRIX_K+j;
	memset((void*)C, 0, sizeof(MATRIX_M * MATRIX_K * sizeof(float)));

	for (int i = 0; i < MATRIX_M; i++)
	{
		for (int j = 0; j < MATRIX_K; j++)
		{
			C[i][j] = 0;
			for (int k = 0; k < MATRIX_N; k++)
			{
				C[i][j] += A[i][k] * B[k][j];
			}
		}
	}

	//openCL
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
    cl_mem clBufA = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        MATRIX_M * MATRIX_N * sizeof(cl_float), A, NULL);
    cl_mem clBufB = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        MATRIX_N * MATRIX_K * sizeof(cl_float), B, NULL);

    cl_mem clBufC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, MATRIX_M * MATRIX_K * sizeof(cl_float), NULL, NULL);

    //kernel文件为add.cl
    const char* filename = "matrix_mul.cl";
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
    cl_kernel kernel = clCreateKernel(program, "simpleMultiply", NULL);
    //设置kernel参数
    int wA = MATRIX_N;
    int hA = MATRIX_M;
    int wB = MATRIX_K;
    int hB = MATRIX_N;
    clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&clBufC);
    clSetKernelArg(kernel, 1, sizeof(cl_int), (void*)&wA);
    clSetKernelArg(kernel, 2, sizeof(cl_int), (void*)&hA);
    clSetKernelArg(kernel, 3, sizeof(cl_int), (void*)&wB);
    clSetKernelArg(kernel, 4, sizeof(cl_int), (void*)&hB);
    clSetKernelArg(kernel, 5, sizeof(cl_mem), (void*)&clBufA);
    clSetKernelArg(kernel, 6, sizeof(cl_mem), (void*)&clBufB);
    //执行kernel， range用1维， work items size为BUFSIZE
    cl_event ev;
    size_t globalws[2] = { MATRIX_M , MATRIX_K };
    clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, globalws, NULL, 0, NULL, &ev);
    clWaitForEvents(1, &ev);

    //数据拷回host内存
    cl_float* ptr = NULL;
    cl_event mapevt;
    ptr = (cl_float*)clEnqueueMapBuffer(command_queue,
        clBufC,
        CL_TRUE,
        CL_MAP_READ,
        0,
        MATRIX_M * MATRIX_K * sizeof(cl_float),
        0, NULL, &mapevt, NULL);
    clWaitForEvents(1, &mapevt);

    //结果验证，和CPU计算的结果比较
    if (!memcmp(C, ptr, MATRIX_M * MATRIX_K * sizeof(cl_float)))
        printf("Verify passed!\n");
    else    printf("Verify failed!\n");

    clReleaseMemObject(clBufC);
    clReleaseMemObject(clBufA);
    clReleaseMemObject(clBufB);
    clReleaseProgram(program);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);

	return 0;
}