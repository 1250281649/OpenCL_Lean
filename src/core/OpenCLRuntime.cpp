#include "OpenCLRuntime.h"
#include "CL/cl.hpp"

#define CHECK_CL_SUCCESS(error) \
	if(error != CL_SUCCESS) {	\
		printf("Error Code %s : %d:%d\n", __func__, __LINE__, (int)error); \
	}

namespace OCL {

OpenCLRuntime::OpenCLRuntime()
{
	std::vector<cl::Platform> platforms;
	cl_int res = cl::Platform::get(&platforms);
	CHECK_CL_SUCCESS(res);
	printf("本机支持OpenCL的环境数量: %d\n", platforms.size());
	if (platforms.size() <= 0)
	{
		mIsCreateError = true;
		printf("Error: no opencl device\n");
		return;
	}

	std::vector<cl::Device> gpuDevices;
	platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &gpuDevices);
	printf("GPU devices num: %d\n", gpuDevices.size());

	mDevice = std::make_shared<cl::Device>(gpuDevices[0]);
	const std::string deviceName = mDevice->getInfo<CL_DEVICE_NAME>();
	printf("GPU name:%s\n", deviceName.c_str());
	const std::string deviceVersion = mDevice->getInfo<CL_DEVICE_VERSION>();
	printf("GPU version:%s\n", deviceVersion.c_str());
	const std::string deviceVendor = mDevice->getInfo<CL_DEVICE_VENDOR>();
	printf("GPU vendor:%s\n", deviceVendor.c_str());

	cl_command_queue_properties properties = 0;
	cl_int err;
	if (deviceVendor.find("NVIDIA") != std::string::npos)
	{
		mGpuType = OCL::NV;
	}
	else {
		mGpuType = OCL::OTHER;
	}
	const std::string extensions = platforms[0].getInfo<CL_PLATFORM_EXTENSIONS>();
	printf("GPU properties:%s\n", extensions.c_str());
	mContext = std::shared_ptr<cl::Context>(new cl::Context({ *mDevice }, nullptr, nullptr, nullptr, &err));
	CHECK_CL_SUCCESS(err);
	mCommandQueue = std::make_shared<cl::CommandQueue>(*mContext, *mDevice, properties, &err);
	CHECK_CL_SUCCESS(err);

	mDevice->getInfo(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, &mGPUGlobalMemeryCacheSize);
	mDevice->getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &mGPUComputeUnits);
	mDevice->getInfo(CL_DEVICE_MAX_CLOCK_FREQUENCY, &mMaxFreq);
	printf("Cache Size:%lld\n", mGPUGlobalMemeryCacheSize);
	printf("Compute units:%d\n", mGPUComputeUnits);
	printf("MaxFreq:%d\n", mMaxFreq);

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

} // namespace OCL