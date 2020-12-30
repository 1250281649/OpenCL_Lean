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
enum GpuType {
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

private:
	std::shared_ptr<cl::Context> mContext;
	std::shared_ptr<cl::Device> mDevice; // default to the first device
	std::shared_ptr<cl::CommandQueue> mCommandQueue;
	uint64_t mGPUGlobalMemeryCacheSize;
	uint32_t mGPUComputeUnits;
	uint32_t mMaxFreq;
	bool mIsSupportedFP16 = false;
	std::map<std::pair<std::string, std::string>, cl::Program> mBuildProgram;
	GpuType mGpuType;	// GPU type
	bool mIsCreateError{false};
};

} // namespace OCL

#endif
