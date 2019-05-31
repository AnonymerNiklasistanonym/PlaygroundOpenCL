#pragma GCC diagnostic ignored "-Wignored-attributes"

#define USE_OPENCL_1_2

#ifndef USE_OPENCL_1_2
// TARGET OPENCL 2.0
#include <CL/cl2.hpp>
#else
// TARGET OPENCL 1.2
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.hpp>
#endif

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

	std::cout << "All OpenCL platforms:" << std::endl;
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

		std::cout << "\tAll OpenCL devices:" << std::endl;
		int deviceCounter = 0;
		for (auto& device : devices) {
			// List all device attributes
			std::cout << "\t" << deviceCounter++
				<< "\tName: " << device.getInfo<CL_DEVICE_NAME>()
				<< "\n\t\tType: " << ((device.getInfo<CL_DEVICE_TYPE>() == CL_DEVICE_TYPE_GPU) ? "GPU" : "CPU")
				<< "\n\t\tVendor: " << device.getInfo<CL_DEVICE_VENDOR>()
				<< "\n\t\tVersion: " << device.getInfo<CL_DEVICE_VERSION>()
				<< "\n\t\tAvailable: " << device.getInfo<CL_DEVICE_AVAILABLE>()
				<< "\n\t\tMax compute units: " << device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>()
				<< "\n\t\tMax clock frequency: " << device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>()
				<< "\n\t\tGlobal memory size: " << (double)device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() / pow(2, 30) << "GB"
				<< "\n\t\tLocal memory size: " << (double)device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>() / pow(2, 10) << "KB"
				<< "\n\t\tMaximum allocatable memory: " << (double)device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() / pow(2, 30) << "GB" << std::endl;
		}
	}

}
