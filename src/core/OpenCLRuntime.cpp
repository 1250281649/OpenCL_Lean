#include <fstream>
#include <sstream>
#include "OpenCLRuntime.h"
#include "CL/cl.hpp"

#define CHECK_CL_SUCCESS(error) \
	if(error != CL_SUCCESS) {	\
		printf("Error Code %s : %d:%d\n", __func__, __LINE__, (int)error); \
	}

namespace OCL {

OpenCLRuntime::OpenCLRuntime()
{
	std::vector<cl::Platform> platformLists;
	cl_int res = cl::Platform::get(&platformLists);
	CHECK_CL_SUCCESS(res);
	printf("本机支持OpenCL的环境数量: %d\n", platformLists.size());
	if (platformLists.size() <= 0)
	{
		mIsCreateError = true;
		printf("Error: no opencl device\n");
		return;
	}

	std::vector<cl::Device> gpuDevices;
	platformLists[0].getDevices(CL_DEVICE_TYPE_GPU, &gpuDevices);
	printf("%-20s: %d\n", "GPU devices num", gpuDevices.size());

	mDevice = std::make_shared<cl::Device>(gpuDevices[0]);
	const std::string deviceName = mDevice->getInfo<CL_DEVICE_NAME>();
	printf("%-20s:%s\n", "GPU name", deviceName.c_str());
	const std::string deviceVersion = mDevice->getInfo<CL_DEVICE_VERSION>();
	printf("%-20s:%s\n", "GPU version", deviceVersion.c_str());
	const std::string deviceVendor = mDevice->getInfo<CL_DEVICE_VENDOR>();
	printf("%-20s:%s\n", "GPU vendor", deviceVendor.c_str());

	cl_command_queue_properties properties = 0;
	cl_int err;
	mGpuType = OCL::GpuType::NV;
	if (deviceVendor.find("NVIDIA") != std::string::npos)
	{
		mGpuType = OCL::GpuType::NV;
	}
	else {
		mGpuType = OCL::GpuType::OTHER;
	}
	const std::string extensions = platformLists[0].getInfo<CL_PLATFORM_EXTENSIONS>();
	printf("%-20s:%s\n", "GPU properties", extensions.c_str());
	mContext = std::shared_ptr<cl::Context>(new cl::Context({ *mDevice }, nullptr, nullptr, nullptr, &err));
	CHECK_CL_SUCCESS(err);
	mCommandQueue = std::make_shared<cl::CommandQueue>(*mContext, *mDevice, properties, &err);
	CHECK_CL_SUCCESS(err);

	mDevice->getInfo(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, &mGPUGlobalMemeryCacheSize);
	mDevice->getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &mGPUComputeUnits);
	mDevice->getInfo(CL_DEVICE_MAX_CLOCK_FREQUENCY, &mMaxFreq);
	printf("%-20s:%lld\n", "Cache Size", mGPUGlobalMemeryCacheSize);
	printf("%-20s:%d\n", "Compute units", mGPUComputeUnits);
	printf("%-20s:%d\n", "MaxFreq", mMaxFreq);

	cl_device_fp_config fpConfig;
	err = mDevice->getInfo(CL_DEVICE_HALF_FP_CONFIG, &fpConfig);
	mIsSupportedFP16 = err == CL_SUCCESS && fpConfig > 0;
}

OpenCLRuntime::~OpenCLRuntime()
{
	mBuildProgram.clear();
	mCommandQueue.reset();
	mContext.reset();
	mDevice.reset();
}

GpuType OpenCLRuntime::getGpuType()
{
	return mGpuType;
}

bool OpenCLRuntime::isSupportedFP16()
{
	return mIsSupportedFP16;
}

cl::Context& OpenCLRuntime::context()
{
	return *mContext;
}

cl::CommandQueue& OpenCLRuntime::commandQueue()
{
	return *mCommandQueue;
}

uint64_t OpenCLRuntime::deviceGlobalMemoryCacheSize()
{
	return mGPUGlobalMemeryCacheSize;
}

uint32_t OpenCLRuntime::deviceComputeUnits()
{
	return mGPUComputeUnits;
}

uint32_t OpenCLRuntime::maxFreq()
{
	return mMaxFreq;
}

bool OpenCLRuntime::loadProgram(const std::string& programName, cl::Program* program)
{
	cl_int errNum;
	
	std::ifstream kernelFile(programName.c_str(), std::ios::in);
	if (!kernelFile.is_open())
	{
		printf("Failed to open file for reading:%s\n", programName.c_str());
		return false;
	}

	std::ostringstream oss;
	oss << kernelFile.rdbuf();
	std::string srcStdStr = oss.str();
	
	cl::Program::Sources sources;
	sources.push_back(std::make_pair(srcStdStr.c_str(), srcStdStr.length()));
	*program = cl::Program(context(), sources);
	if (program == nullptr)
	{
		printf("Error: failed to create CL program from source\n");
		return false;
	}
	oss.clear();
	kernelFile.close();

	return true;
}

bool OpenCLRuntime::buildProgram(cl::Program* program, const std::string& buildOptions)
{
	cl_int ret = program->build({*mDevice}, buildOptions.c_str());
	if (ret != CL_SUCCESS)
	{
		if (program->getBuildInfo<CL_PROGRAM_BUILD_STATUS>(*mDevice) == CL_BUILD_ERROR)
		{
			std::string buildLog = program->getBuildInfo<CL_PROGRAM_BUILD_LOG>(*mDevice);
			printf("Program build log: %s\n", buildLog.c_str());
		}
		printf("Build program failed!\n");
		return false;
	}
	return true;
}

cl::Kernel OpenCLRuntime::buildKernel(const std::string& programName, const std::string& kernelName,
	const std::set<std::string>& buildOptions)
{
	cl::Kernel kernel;
	cl::Program program;
	if (!this->loadProgram(programName, &program)) {
		return kernel;
	}

	std::string Options;
	if (!this->buildProgram(&program, Options)) {
		return kernel;
	}

	cl_int err;
	kernel = cl::Kernel(program, kernelName.c_str(), &err);
	CHECK_CL_SUCCESS(err);
	return kernel;
}

} // namespace OCL