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

void calculateHost(const std::vector<int>& host_input, std::vector<int>& host_output)
{
	for (std::size_t i = 0; i < host_output.size(); i++) {
		host_output[i] = host_input[i] * 2;
	}

}

int main()
{
	// This is a demo of where the strengths of OpenCL lies:
	// Its the simples one: Multiply in a very long list of numbers, every number times itself

	// Create big input array:
	constexpr unsigned int arraySize = 4; // std::numeric_limits<double>::max() / 1000;
	std::cout << "Create input and output arrays with the size of: " << arraySize << std::endl;
	std::vector<int> input(arraySize);
	std::vector<int> host_output(arraySize);
	std::vector<int> device_output(arraySize);
	
	// Fill it with random value:
	constexpr int min = std::numeric_limits<int>::lowest() / 2 + 1;
	constexpr int max = std::numeric_limits<int>::max() / 2 - 1;
	// Create (seed) engine
	std::random_device rd;
	// The random-number engine (Mersenne-Twister)
	std::mt19937 rng(rd());   
	// Initialize guaranteed unbiased random number generator
	std::uniform_int_distribution<int> uni(min, max);
	std::cout << "Fill the array with random values between " << min << " and " << max << std::endl;
	for (auto& element : input) {
		element = uni(rng);
	}

	// Run the host code
	std::cout << "Run the host code" << std::endl;
	const std::chrono::steady_clock::time_point cpu_calculation_begin = std::chrono::steady_clock::now();
	calculateHost(input, host_output);
	const std::chrono::steady_clock::time_point cpu_calculation_end = std::chrono::steady_clock::now();

	const auto cpu_calculation_time_nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(cpu_calculation_end - cpu_calculation_begin).count();
	std::cout << "CPU time:\n\tCalculation: " << cpu_calculation_time_nanoseconds << "ns" << std::endl;


	// Now run the same code on the iGPU

	std::vector<cl::Platform> platforms;
	// Get all platforms
	cl::Platform::get(&platforms);
	// Check if there is a platform
	if (platforms.size() == 0) {
		std::cerr << "No OpenCL platforms found" << std::endl;
		return EXIT_FAILURE;
	}
	// Find the correct platform index
	int platformId = -1;
	for (std::size_t i = 0; i < platforms.size(); i++) {
		std::string currentPlatform = platforms[i].getInfo<CL_PLATFORM_NAME>();
		currentPlatform = currentPlatform.substr(0, currentPlatform.size() - 1);
		if (currentPlatform == "Intel(R) OpenCL") {
			platformId = i;
			break;
		}
	}
	if (platformId < 0) {
		std::cerr << "No Intel OpenCL platform found" << std::endl;
		return EXIT_FAILURE;
	}
	const cl::Platform platform = platforms[platformId];
	std::cout << "Using platform " << platform.getInfo<CL_PLATFORM_NAME>()
		<< "from " << platform.getInfo<CL_PLATFORM_VENDOR>()
		<< "with OpenCL version: " << platform.getInfo<CL_PLATFORM_VERSION>() << std::endl;

	// Get all GPU devices of the platform
	std::vector<cl::Device> devices;
	platforms[platformId].getDevices(CL_DEVICE_TYPE_GPU, &devices);
	// Check if there is a device
	if (devices.size() == 0) {
		std::cerr << "No OpenCL devices found" << std::endl;
		return EXIT_FAILURE;
	}
	// Find the iGPU
	int deviceId = -1;
	for (std::size_t i = 0; i < devices.size(); i++) {
		std::string currentDevice = devices[i].getInfo<CL_DEVICE_NAME>();
		currentDevice = currentDevice.substr(0, currentDevice.size() - 1);
		if (currentDevice == "Intel(R) Iris(TM) Graphics 550") {
			deviceId = i;
			break;
		}
	}
	if (deviceId < 0) {
		std::cerr << "No Intel(R) Iris(TM) Graphics 550 OpenCL device found" << std::endl;
		return EXIT_FAILURE;
	}
	const cl::Device device = devices[deviceId];
	const int maxComputeUnits = device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
	std::cout << "Using device " << device.getInfo<CL_DEVICE_NAME>()
		<< "from " << device.getInfo<CL_DEVICE_VENDOR>()
		<< "with OpenCL version: " << device.getInfo<CL_DEVICE_VERSION>()
		<< "and support for up to " << maxComputeUnits << " compute units" << std::endl;

	// Create a context
	cl_int err;
	cl::Context context({ device }, nullptr, nullptr, nullptr, &err);
	if (err != CL_SUCCESS) {
		std::cout << "Context::Context failed - " << err << std::endl;
		std::cin.get();
	}
	// Read in the kernel from the kernel file
	std::ifstream kerneFile("kernel.cl");
	std::string kernelContent((std::istreambuf_iterator<char>(kerneFile)), (std::istreambuf_iterator<char>()));
	cl::Program::Sources sources;
	sources.push_back({ kernelContent.c_str(), kernelContent.length() });
	// Build it
	cl::Program program(context, sources, &err);
	if (err != CL_SUCCESS) {
		std::cout << "Program::Program failed - " << err << std::endl;
		std::cin.get();
	}
	auto programBuildError = program.build("-cl-std=CL1.2");
	// Check if building the kerne was successful
	if (programBuildError != CL_SUCCESS) {
		std::cout << "Program build error";
		return EXIT_FAILURE;
	}

	// Create buffer for input and output
	const int bufferSize = sizeof(cl_int) * arraySize;
	cl::Buffer device_input_buffer(context, CL_MEM_READ_WRITE, bufferSize);
	cl::Buffer device_output_buffer(context, CL_MEM_READ_WRITE, bufferSize);
	
	// Create a command queue
	cl::CommandQueue queue(context, device);

	// Fill device input buffer with input vector
	queue.enqueueWriteBuffer(device_input_buffer, true, 0, bufferSize, input.data());
	queue.enqueueWriteBuffer(device_output_buffer, true, 0, bufferSize, device_output.data());

	// Create kernel
	cl::Kernel kernel(program, "kernelSimple", &err);
	if (err != CL_SUCCESS) {
		std::cout << "Kernel::Kernel failed - " << err << std::endl;
		std::cin.get();
	}
	// Set the arguments for the kernel
	kernel.setArg(0, device_input_buffer);
	kernel.setArg(1, device_output_buffer);

	cl::Event eventGpuCalculation;
	cl::Event eventGpuOutWrite;

	// Launch the kernel
	queue.enqueueNDRangeKernel(kernel, 0, cl::NDRange(arraySize), cl::NDRange(maxComputeUnits), NULL, &eventGpuCalculation);

	// Read out what the device wrote in memory
	queue.enqueueReadBuffer(device_output_buffer, true, 0, bufferSize, device_output.data(), NULL, &eventGpuOutWrite);

	// Output results
	/*
	for (std::size_t i = 0; i < input.size(); i++) {
		std::cout << i << "\t" << "input: " << input[i] << "\thost: " << host_output[i] << "\tdevice: " << device_output[i] << std::endl;
	}*/
}

