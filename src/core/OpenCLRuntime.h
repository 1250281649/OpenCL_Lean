// 
// OpenCLRuntime.h
//
#ifndef OpenCLRuntime_h
#define OpenCLRuntime_h

#include <vector>
#include <map>
#include <set>
#include <string>
#include <CL/cl.hpp>

namespace OCL {
enum class GpuType {
		MALI = 0, 
		NV = 1,
		ADRENO = 2,
		OTHER=3
};

class OpenCLRuntime {
public:
	OpenCLRuntime();
	~OpenCLRuntime();
	OpenCLRuntime(const OpenCLRuntime&) = delete;
	OpenCLRuntime& operator=(const OpenCLRuntime&) = delete;

	GpuType getGpuType();
	bool isSupportedFP16();
	cl::Context& context();
	cl::CommandQueue& commandQueue();
	uint64_t deviceGlobalMemoryCacheSize();
	uint32_t deviceComputeUnits();
	uint32_t maxFreq();
	cl::Kernel buildKernel(const std::string& programName, const std::string& kernelName,
		const std::set<std::string>& buildOptions);

private:
	bool loadProgram(const std::string& programName, cl::Program* program);
	bool buildProgram(cl::Program* program, const std::string& buildOptions);

private:
	std::shared_ptr<cl::Context> mContext;
	std::shared_ptr<cl::Device> mDevice; // default to the first device
	std::shared_ptr<cl::CommandQueue> mCommandQueue;
	uint64_t mGPUGlobalMemeryCacheSize = 0;
	uint32_t mGPUComputeUnits = 0;
	uint32_t mMaxFreq = 0;
	bool mIsSupportedFP16 = false;
	std::map<std::pair<std::string, std::string>, cl::Program> mBuildProgram;
	GpuType mGpuType = GpuType::OTHER;	// GPU type
	bool mIsCreateError{false};
};

} // namespace OCL

#endif
