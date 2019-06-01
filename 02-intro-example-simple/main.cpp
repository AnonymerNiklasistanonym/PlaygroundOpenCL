// For CMake
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
#include <string>
#include <limits>
#include <fstream>
#include <random>
#include <chrono>

int compute_factorial(const int& n)
{
	int factorial = 1;

	for (int i = 1; i <= n; i++)
	{
		factorial *= i;
	}
	return factorial;
}


void calculateHost(std::vector<int>& host_output, const unsigned int& factorial)
{
	for (std::size_t i = 0; i < host_output.size(); i++) {
		host_output[i] = static_cast<int>(i) + compute_factorial(factorial);
	}

}

int main()
{
	// This is a demo of how to write a simple kernel in OpenCL
	// Its the simples one: Fill an array with its indices at every element and add a factorial of 10

	// Create big input array:
	constexpr unsigned int MAXIMUM_WORG_GROUP_SIZE_INTEL = 16;
	constexpr unsigned int wgSizeX = MAXIMUM_WORG_GROUP_SIZE_INTEL;
	constexpr unsigned int wgSizeY = MAXIMUM_WORG_GROUP_SIZE_INTEL;
	constexpr unsigned int countX = wgSizeX * 100;
	constexpr unsigned int countY = wgSizeY * 100;
	constexpr unsigned int count = countX * countY;
	constexpr unsigned int size = count * sizeof(cl_int);
	constexpr unsigned int factorial = 100;
	std::cout << "Create input and output arrays with the size of: " << count << std::endl;
	std::vector<int> host_output(count);
	std::vector<int> device_output(count);

	// -------------------
	// | CPU / HOST      |
	// -------------------

	// Run the host code
	std::cout << "Run the host code" << std::endl;
	const std::chrono::steady_clock::time_point cpu_calculation_begin = std::chrono::steady_clock::now();
	calculateHost(host_output, factorial);
	const std::chrono::steady_clock::time_point cpu_calculation_end = std::chrono::steady_clock::now();

	// Calculate duration
	const auto cpu_calculation_time_nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(cpu_calculation_end - cpu_calculation_begin).count();
	std::cout << "CPU time:\n\tCalculation: " << cpu_calculation_time_nanoseconds << "ns" << std::endl;
	const auto cpu_time_nanoseconds = cpu_calculation_time_nanoseconds;

	// -------------------
	// | GPU / DEVICE    |
	// -------------------

	const char* platformName = "Intel(R) OpenCL";
	const char* deviceName = "Intel(R) Iris(TM) Graphics 550";

	// Get all platforms
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	// Check if there is a platform
	if (platforms.size() == 0) {
		std::cerr << "No OpenCL platforms found" << std::endl;
		return EXIT_FAILURE;
	}
	// Find the correct platform index
	std::size_t platformId = -1;
	for (std::size_t i = 0; i < platforms.size(); i++) {
		std::string currentPlatform = platforms[i].getInfo<CL_PLATFORM_NAME>();
		currentPlatform = currentPlatform.substr(0, currentPlatform.size() - 1);
		if (currentPlatform == platformName) {
			platformId = i;
			break;
		}
	}
	if (platformId < 0) {
		std::cerr << "No \"" << platformName << "\" OpenCL platform found" << std::endl;
		return EXIT_FAILURE;
	}
	const cl::Platform platform = platforms[platformId];
	std::cout << "Using platform " << platform.getInfo<CL_PLATFORM_NAME>()
		<< " from " << platform.getInfo<CL_PLATFORM_VENDOR>()
		<< " with OpenCL version: " << platform.getInfo<CL_PLATFORM_VERSION>() << std::endl;

	// Get all GPU devices of the platform
	std::vector<cl::Device> devices;
	platforms[platformId].getDevices(CL_DEVICE_TYPE_GPU, &devices);
	// Check if there is a device
	if (devices.size() == 0) {
		std::cerr << "No OpenCL devices found" << std::endl;
		return EXIT_FAILURE;
	}
	// Find the iGPU
	std::size_t deviceId = -1;
	for (std::size_t i = 0; i < devices.size(); i++) {
		std::string currentDevice = devices[i].getInfo<CL_DEVICE_NAME>();
		currentDevice = currentDevice.substr(0, currentDevice.size() - 1);
		if (currentDevice == deviceName) {
			deviceId = i;
			break;
		}
	}
	if (deviceId < 0) {
		std::cerr << "No \"" << deviceName << "\"  OpenCL device found" << std::endl;
		return EXIT_FAILURE;
	}
	const cl::Device device = devices[deviceId];
	std::cout << "Using device " << device.getInfo<CL_DEVICE_NAME>()
		<< "from " << device.getInfo<CL_DEVICE_VENDOR>()
		<< "with OpenCL version: " << device.getInfo<CL_DEVICE_VERSION>()
		<< "and the maximum work group size is " << device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>()
		<< " (currently using wgSizeX=" << wgSizeX << "x" <<wgSizeY <<"=wgSizeY = " << (wgSizeX * wgSizeY) << " workgroups)" << std::endl;

	// Create a context
	cl_int err;
	cl::Context context({ device }, nullptr, nullptr, nullptr, &err);
	if (err != CL_SUCCESS) {
		std::cerr << "Context::Context failed" << std::endl;
		std::cin.get();
		return EXIT_FAILURE;
	}
	// Read in the kernel from the kernel file
	std::ifstream kerneFile("kernel.cl");
	std::string kernelContent((std::istreambuf_iterator<char>(kerneFile)), (std::istreambuf_iterator<char>()));
	cl::Program::Sources sources;
	sources.push_back({ kernelContent.c_str(), kernelContent.length() });
	// Build it
	cl::Program program(context, sources, &err);
	if (err != CL_SUCCESS) {
		std::cerr << "Program::Program failed" << std::endl;
		std::cin.get();
		return EXIT_FAILURE;
	}
	auto programBuildError = program.build("-cl-std=CL1.2");
	// Check if building the kerne was successful
	if (programBuildError != CL_SUCCESS) {
		std::cerr << "Program build error" << std::endl;
		std::cin.get();
		return EXIT_FAILURE;
	}

	// Create buffer for output
	cl::Buffer device_output_buffer(context, CL_MEM_READ_WRITE, size);

	// Create a command queue
	cl::CommandQueue queue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
	if (err != CL_SUCCESS) {
		std::cerr << "CommandQueue error" << std::endl;
		std::cin.get();
		return EXIT_FAILURE;
	}

	// Create kernel
	cl::Kernel kernel(program, "kernelSimple", &err);
	if (err != CL_SUCCESS) {
		std::cerr << "Kernel::Kernel failed" << std::endl;
		std::cin.get();
		return EXIT_FAILURE;
	}
	// Set the arguments for the kernel
	kernel.setArg<cl::Buffer>(0, device_output_buffer);
	kernel.setArg<cl_uint>(1, factorial);

	// Launch the kernel
	cl::Event eventGpuCalculation;
	err = queue.enqueueNDRangeKernel(kernel, 0, cl::NDRange(countX, countY), cl::NDRange(wgSizeX, wgSizeY), NULL, &eventGpuCalculation);
	if (err != CL_SUCCESS) {
		std::cerr << "queue::enqueueNDRangeKernel failed" << std::endl;
		std::cin.get();
		return EXIT_FAILURE;
	}
	const auto gpu_calculation_time_nanoseconds = eventGpuCalculation.getProfilingInfo<CL_PROFILING_COMMAND_END>() - eventGpuCalculation.getProfilingInfo<CL_PROFILING_COMMAND_START>();

	// Read out what the device wrote in memory
	cl::Event eventGpuWriteBack;
	err = queue.enqueueReadBuffer(device_output_buffer, true, 0, size, device_output.data(), NULL, &eventGpuWriteBack);
	if (err != CL_SUCCESS) {
		std::cerr << "queue::enqueueReadBuffer failed" << std::endl;
		std::cin.get();
		return EXIT_FAILURE;
	}
	const auto gpu_write_back_time_nanoseconds = eventGpuWriteBack.getProfilingInfo<CL_PROFILING_COMMAND_END>() - eventGpuWriteBack.getProfilingInfo<CL_PROFILING_COMMAND_START>();
	const auto opencl_time_nanoseconds = gpu_calculation_time_nanoseconds + gpu_write_back_time_nanoseconds;

	std::cout << "GPU time:\n\tCalculation: " << gpu_calculation_time_nanoseconds << "ns\n\tWrite back: " << gpu_write_back_time_nanoseconds << "ns" << std::endl;

	// Check results
	unsigned int errors = 0;
	for (std::size_t i = 0; i < host_output.size(); i++) {
		if (host_output[i] != device_output[i]) {
			errors++;
			if (errors < 10) {
				std::cout << "Error at " << i << " -> CPU: " << host_output[i]  << ", GPU: " << device_output[i] << std::endl;
			}
			if (errors == 10) {
				std::cout << "..." << std::endl;
			}
		}
	}

	// Give back results and speed comparison
	if (errors == 0) {
		std::cout << "Success, all values are correct" << std::endl;
		// TODO: There is currently no speed up, check this later on a PC with a real GPU?
		// TODO: Kernel event time is in Visual Studio 0 and in CMake executable really big
		std::cout << "Speed comparison: CPU=" << cpu_time_nanoseconds
			<< "ns vs GPU=" << opencl_time_nanoseconds
			<< "ns (=> Speedup: " << ((double)cpu_time_nanoseconds / opencl_time_nanoseconds) << ")" << std::endl;
		return EXIT_SUCCESS;
	}
	else {
		std::cout << "Failure, errors between GPU and CPU: " << errors << std::endl;
		return EXIT_FAILURE;
	}
}

