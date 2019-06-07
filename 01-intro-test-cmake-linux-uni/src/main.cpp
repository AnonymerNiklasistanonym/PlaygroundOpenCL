// For CMake
// #pragma GCC diagnostic ignored "-Wignored-attributes"

// TARGET OPENCL 2.0
#define CL_HPP_TARGET_OPENCL_VERSION 200
#include <CL/cl2.hpp>

#include <cmath>
#include <vector>
#include <iostream>

bool runKernelOnOpenClDevice(cl::Device &device);

int main()
{
    std::cout << "Hello World!" << std::endl;

    // List all devices that support OpenCL on this system
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if (platforms.size() <= 0)
    {
        std::cerr << "No supported plaforms found!";
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << platforms.size() << " platform(s) found" << std::endl;
    }

    std::cout << "\033[1;34mAll OpenCL platforms:\033[0m" << std::endl;
    int platformCounter = 0;
    for (auto &platform : platforms)
    {
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
        for (auto &device : devices)
        {
            // List all device attributes
            std::cout << "\t" << deviceCounter++
                      << "\tName: " << device.getInfo<CL_DEVICE_NAME>()
                      << "\n\t\tType: " << ((device.getInfo<CL_DEVICE_TYPE>() == CL_DEVICE_TYPE_GPU) ? "\033[1;32mGPU\033[0m" : "\033[1;31mCPU\033[0m")
                      << "\n\t\tVendor: " << device.getInfo<CL_DEVICE_VENDOR>()
                      << "\n\t\tVersion: " << device.getInfo<CL_DEVICE_VERSION>()
                      << "\n\t\tAvailable: " << device.getInfo<CL_DEVICE_AVAILABLE>()
                      << "\n\t\tMax work group size: " << device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>()
                      << "\n\t\tMax clock frequency: " << device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>()
                      << "\n\t\tGlobal memory size: " << (double)device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() / 1024 / 1024 / 1024 << "GB"
                      << "\n\t\tLocal memory size: " << (double)device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>() / 1024 << "KB"
                      << "\n\t\tMaximum allocatable memory: " << (double)device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() / 1024 / 1024 / 1024 << "GB" << std::endl;

            // Run on device an example kernel
            if (!runKernelOnOpenClDevice(device)) {
                std::cout << "\t\033[1;31mError running the kernel!\033[0m" << std::endl;
            }
        }
    }
}

bool runKernelOnOpenClDevice(cl::Device &device)
{
    if (!device.getInfo<CL_DEVICE_AVAILABLE>())
    {
        std::cout << "\t\tDevice is not available" << std::endl;
        return true;
    }
    std::cout << "\t\t>> Run example kernel on OpenCL device \"" << device.getInfo<CL_DEVICE_NAME>() << "\"" << std::endl;

    // a context is like a "runtime link" to the device and platform;
    // i.e. communication is possible
    cl::Context context({device});

    // create the program that we want to execute on the device
    cl::Program::Sources sources;

    // calculates for each element; C = A + B
    std::string kernel_code =
        "void kernel simple(global int* output) {"
        "    const uint countX = get_global_id(0);"
        "    output[countX] = countX;"
        "}";
    sources.push_back({kernel_code.c_str(), kernel_code.length()});

    cl::Program program(context, sources);
    if (program.build({device}) != CL_SUCCESS)
    {
        std::cout << "\t\t\033[1;31mError building:\033[0m " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
        return false;
    }

    const unsigned int size = 1000;

    // apparently OpenCL only likes arrays ...
    // N holds the number of elements in the vectors we want to add
    std::vector<cl_int> vec_output(size);
    memset(vec_output.data(), 100, size);

    // create buffers on device (allocate space on GPU)
    cl::Buffer buffer_output(context, CL_MEM_READ_ONLY, sizeof(cl_int) * size);

    // create a queue (a queue of commands that the GPU will execute)
    cl::CommandQueue queue(context, device, CL_QUEUE_PROFILING_ENABLE);

    // push write commands to queue
    queue.enqueueWriteBuffer(buffer_output, CL_TRUE, 0, sizeof(cl_int) * size, vec_output.data());

    // RUN ZE KERNEL
    cl::Kernel kernel_simple = cl::Kernel(program, "simple");
    kernel_simple.setArg(0, buffer_output);
    queue.enqueueNDRangeKernel(kernel_simple, cl::NullRange, cl::NDRange(size), cl::NullRange);
    queue.finish();

    // read result from GPU to here
    queue.enqueueReadBuffer(buffer_output, CL_TRUE, 0, sizeof(cl_int) * size, vec_output.data());
    queue.finish();

    for (unsigned int i = 0; i < vec_output.size(); i++)
    {
        if (static_cast<int>(i) != vec_output[i]) {
            std::cout << "\t\t\033[1;31mError in kernel execution!\033[0m" << std::endl;
            return false;
        }
    }

    return true;
}