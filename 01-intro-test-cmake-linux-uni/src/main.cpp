// For CMake
// #pragma GCC diagnostic ignored "-Wignored-attributes"

// TARGET OPENCL 2.0
#define CL_HPP_TARGET_OPENCL_VERSION 200
#include <CL/cl2.hpp>

#include <cmath>
#include <vector>
#include <iostream>

int main()
{
	std::cout << "Hello World!" << std::endl;

	// List all devices that support OpenCL on this system
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);

	if (platforms.size() <= 0) {
		std::cerr << "No supported plaforms found!";
		return EXIT_FAILURE;
	}
	else {
		std::cout << platforms.size() << " platform(s) found" << std::endl;
	}

	std::cout << "\033[1;34mAll OpenCL platforms:\033[0m" << std::endl;
	int platformCounter = 0;
	for (auto& platform : platforms) {
		// List all platform attributes
		std::cout << platformCounter++
			<< "\tName: " << platform.getInfo<CL_PLATFORM_NAME>()
			<< "\n\tVendor: " << platform.getInfo<CL_PLATFORM_VENDOR>()
			<< "\n\tVersion: " << platform.getInfo<CL_PLATFORM_VERSION>()
			<< "\n\tProfile: " << platform.getInfo<CL_PLATFORM_PROFILE>()
			<< "\n\tICD suffix KHR: " << platform.getInfo<CL_PLATFORM_ICD_SUFFIX_KHR>()
			<< "\n\tExtensions: " << platform.getInfo<CL_PLATFORM_EXTENSIONS>() << std::endl;

		// List all devices of this platform
		std::vector<cl::Device> devices;
		platform.getDevices(CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_CPU, &devices);

		std::cout << "\t\033[1;34mAll OpenCL devices of this platform:\033[0m" << std::endl;
		int deviceCounter = 0;
		for (auto& device : devices) {
			// List all device attributes
			std::cout << "\t" << deviceCounter++
				<< "\tName: " << device.getInfo<CL_DEVICE_NAME>()
				<< "\n\t\tType: " << ((device.getInfo<CL_DEVICE_TYPE>() == CL_DEVICE_TYPE_GPU) ? "\033[1;32mGPU\033[0m" : "\033[1;31mCPU\033[0m")
				<< "\n\t\tVendor: " << device.getInfo<CL_DEVICE_VENDOR>()
				<< "\n\t\tVersion: " << device.getInfo<CL_DEVICE_VERSION>()
				<< "\n\t\tAvailable: " << device.getInfo<CL_DEVICE_AVAILABLE>()
				<< "\n\t\tMax work group size: " << device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>()
				<< "\n\t\tMax clock frequency: " << device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>()
				<< "\n\t\tGlobal memory size: " << (double)device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() / pow(2, 30) << "GB"
				<< "\n\t\tLocal memory size: " << (double)device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>() / pow(2, 10) << "KB"
				<< "\n\t\tMaximum allocatable memory: " << (double)device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() / pow(2, 30) << "GB" << std::endl;
		}
	}

}
