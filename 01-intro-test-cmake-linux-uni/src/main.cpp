// TARGET OPENCL 2.0
#define CL_HPP_TARGET_OPENCL_VERSION 200
#include <CL/cl2.hpp>

// Include c libraries
#include <cmath>

// Include stl libraries
#include <chrono>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

// Define functions that will be used in main but declared below it
void runHostCode(std::vector<int> &outputVector);
const bool runKernelOnOpenClDevice(cl::Device &device, std::vector<int> &outputVector,
                                   const uint64_t &cpuTimeNs);
inline const uint64_t getTimeInNs(cl::Event &openClEvent);
inline const std::string displayTimeAndSpeedup(const uint64_t &timeNs,
                                               const bool &cpuComparison = false, const uint64_t &cpuTimeNs = 0);

// Main method
int main()
{
    // Display hello world and date and time of compilation
    std::cout << "Hello World! (Compiled on " << __DATE__  << " at " << __TIME__  << ")" << std::endl;

    // List all devices that support OpenCL on this system
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if (platforms.size() <= 0) {
        std::cerr << "No supported plaforms found!";
        return EXIT_FAILURE;
    }
    std::cout << platforms.size() << " platform(s) found" << std::endl;

    // Set the size of the array
    const unsigned int size = 100000000;

    // Run the host code
    std::vector<int> vec(size);
    memset(vec.data(), -1, size);
    std::cout << "Run the host code" << std::endl;
    const std::chrono::steady_clock::time_point cpu_calculation_begin =
        std::chrono::steady_clock::now();
    runHostCode(vec);
    const std::chrono::steady_clock::time_point cpu_calculation_end = std::chrono::steady_clock::now();
    const auto cpuTimeNs = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>
                                                 (cpu_calculation_end - cpu_calculation_begin).count());
    std::cout << "\tTime: " << displayTimeAndSpeedup(cpuTimeNs) << std::endl;

    std::cout << "\033[1;34mAll OpenCL platforms:\033[0m" << std::endl;
    int platformCounter = 0;
    for (auto &platform : platforms) {
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
        for (auto &device : devices) {
            // List all device attributes
            std::cout << "\t" << deviceCounter++
                      << "\tName: " << device.getInfo<CL_DEVICE_NAME>()
                      << "\n\t\tType: " << ((device.getInfo<CL_DEVICE_TYPE>() == CL_DEVICE_TYPE_GPU)
                                            ? "\033[1;32mGPU\033[0m" : "\033[1;31mCPU\033[0m")
                      << "\n\t\tVendor: " << device.getInfo<CL_DEVICE_VENDOR>()
                      << "\n\t\tVersion: " << device.getInfo<CL_DEVICE_VERSION>()
                      << "\n\t\tAvailable: " << device.getInfo<CL_DEVICE_AVAILABLE>()
                      << "\n\t\tMax work group size: " << device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>()
                      << "\n\t\tMax clock frequency: " << device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>()
                      << "\n\t\tGlobal memory size: "
                      << device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() / pow(1024.0, 3) << "GB"
                      << "\n\t\tLocal memory size: "
                      << device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>() / pow(1024.0, 1) << "KB"
                      << "\n\t\tMaximum allocatable memory: "
                      << device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() / pow(1024.0, 3) << "GB" << std::endl;

            // Run on device an example kernel
            if (!runKernelOnOpenClDevice(device, vec, cpuTimeNs)) {
                std::cout << "\t\t\033[1;31mError running the kernel!\033[0m" << std::endl;
            }
        }
    }
}

void runHostCode(std::vector<int> &vec)
{
    for (unsigned int i = 0; i < vec.size(); i++) {
        if (static_cast<int>(i) != vec[i]) {
            vec[i] = static_cast<int>(i);
        }
    }
}

const bool runKernelOnOpenClDevice(cl::Device &device, std::vector<int> &vec,
                                   const uint64_t &cpuTimeNs)
{
    if (!device.getInfo<CL_DEVICE_AVAILABLE>()) {
        std::cout << "\t\tDevice is not available" << std::endl;
        return true;
    }
    std::cout << "\t\t>> Run example kernel on OpenCL device \""
              << device.getInfo<CL_DEVICE_NAME>() << "\"" << std::endl;

    // Create link between the device and platform;
    cl::Context context({device});

    // Create the program that should be executed that is equivalent to the host code
    cl::Program::Sources sources;
    std::string kernel_code =
        "void kernel simple(global int* output) {"
        "    const uint countX = get_global_id(0);"
        "    output[countX] = countX;"
        "}";
    sources.push_back({kernel_code.c_str(), kernel_code.length()});
    // Build the program and check if compilation was successful
    cl::Program program(context, sources);
    if (program.build({device}) != CL_SUCCESS) {
        std::cout << "\t\t\033[1;31mError building:\033[0m "
                  << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
        return false;
    }

    // Create vector which we will read and write
    memset(vec.data(), -1, vec.size());

    // Create buffer and check if the size is OK
    const unsigned int bufferSize = sizeof(cl_int) * vec.size();
    const uint64_t maxBufferSize = device.getInfo< CL_DEVICE_MAX_MEM_ALLOC_SIZE>();
    if (bufferSize > maxBufferSize) {
        std::cout << "\t\t\033[1;31mError: The buffer size (" << bufferSize
                  << ") is bigger than the device max mem alloc size (" << maxBufferSize
                  << ")!\033[0m " << std::endl;
        return false;
    }

    // Allocate buffer on the OpenCL device for the vector data
    cl::Buffer buffer_output(context, CL_MEM_READ_ONLY, bufferSize);

    // Create a queue of commands that will be executed on the OpenCL device
    cl::CommandQueue queue(context, device, CL_QUEUE_PROFILING_ENABLE);

    // Write current vector data to the OpenCL buffer
    cl::Event eventWriteToBuffer;
    queue.enqueueWriteBuffer(buffer_output, CL_TRUE, 0, sizeof(cl_int) * vec.size(), vec.data(), NULL,
                             &eventWriteToBuffer);

    // Run the kernel/program on the OpenCL device
    cl::Kernel kernel_simple = cl::Kernel(program, "simple");
    kernel_simple.setArg(0, buffer_output);
    cl::Event eventKernelExecution;
    queue.enqueueNDRangeKernel(kernel_simple, cl::NullRange, cl::NDRange(vec.size()), cl::NullRange,
                               NULL,
                               &eventKernelExecution);

    // Read the new values in the OpenCL device back into to the host into out vector
    cl::Event eventReadFromBuffer;
    queue.enqueueReadBuffer(buffer_output, CL_TRUE, 0, sizeof(cl_int) * vec.size(), vec.data(), NULL,
                            &eventReadFromBuffer);

    // Calculate all times
    const auto writeToBufferNs = getTimeInNs(eventWriteToBuffer);
    const auto kernelExecutionNs = getTimeInNs(eventKernelExecution);
    const auto readFromBufferNs = getTimeInNs(eventReadFromBuffer);
    const auto wholeTimeNs = writeToBufferNs + kernelExecutionNs + readFromBufferNs;
    std::cout << "\t\t\tTime:\n\t\t\t\tWrite to buffer:  " << writeToBufferNs
              << "ns\n\t\t\t\tKernel execution: " << kernelExecutionNs
              << "ns\n\t\t\t\tRead from buffer: " << readFromBufferNs
              << "ns\n\t\t\t\t\t=> Sum:   " << displayTimeAndSpeedup(wholeTimeNs, true, cpuTimeNs) << std::endl;

    // Check if the "calculation" was successful
    unsigned int errorCount = 0;
    // display errors (the first 10 then ...
    // and error count
    for (unsigned int i = 0; i < vec.size(); i++) {
        if (static_cast<int>(i) != vec[i]) {
            if (errorCount < 10) {
                std::cout << "\t\t\t\033[1;31mCalculation error in kernel execution! (" << i << "=!" << vec[i]
                          << ")\033[0m" << std::endl;
            } else if (errorCount == 10) {
                std::cout << "\t\t\t\033[1;31m...\033[0m" << std::endl;
            }
            errorCount++;
        }
        // std::cout << i << ": " << vec_output[i] << std::endl;
    }
    if (errorCount > 0) {
        std::cout << "\t\t\033[1;31mCalculation errors in kernel execution: " << errorCount << "\033[0m"
                  << std::endl;
        return false;
    } else {
        return true;
    }
}

inline const uint64_t getTimeInNs(cl::Event &openClEvent)
{
    return openClEvent.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
           openClEvent.getProfilingInfo<CL_PROFILING_COMMAND_START>();

}

inline const std::string displayTimeAndSpeedup(const uint64_t &timeNs, const bool &cpuComparison,
                                               const uint64_t &cpuTimeNs)
{
    std::stringstream ss;
    ss << timeNs << "ns (" << timeNs / 1000000000.0 << "s";
    if (cpuComparison) {
        ss << ", Speedup: " << static_cast<double>(cpuTimeNs) / timeNs;
    }
    ss << ")";
    return ss.str();
}