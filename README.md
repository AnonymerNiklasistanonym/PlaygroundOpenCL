# Playground OpenCL C++

## Setup

### Windows

#### Intel CPU

1. Go to the [Intel developer zone system studio website](https://dynamicinstaller.intel.com/system-studio/)
2. Search for `opencl` to find the tool `OpenCL Tools for Visual Studio`
3. Add it to the cart and press `Continue to Download`
4. Select `Maybe next time. Please take me to my download.` at the bottom right of the popup window
5. Select the download button in the column `Windows* Host, Windows* Target (Plugin for Visual Studio*)`
6. Then extract the file and run the web based installer
7. Restart your Computer

##### Write a program

```cpp
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
				<< "\n\t\tMax work group size: " << device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>()
				<< "\n\t\tMax clock frequency: " << device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>()
				<< "\n\t\tGlobal memory size: " << (double)device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() / pow(2, 30) << "GB"
				<< "\n\t\tLocal memory size: " << (double)device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>() / pow(2, 10) << "KB"
				<< "\n\t\tMaximum allocatable memory: " << (double)device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() / pow(2, 30) << "GB" << std::endl;
		}
	}

}
```

###### Visual Studio ([01-intro-test-visual-studio](01-intro-test-visual-studio/))

1. Start Visual Studio (Community [2019])
2. Create a new C++ console application
3. Change `Debug`, `x32` to `x64` in the toolbar
4. Select the project (The very first child and second element in the tree view)
5. Go to `C/C++` > `Additional Include directories/Zusätzliche Includeverzeichnisse` and add a path to the include directory of the Intel OpenCL SDK (`C:\Program Files x86\IntelSWTools\OpenCL\sdk\include` which should contain a directory called `CL`)
6. Go to `C/C++` > `Linker` and add a path to the `OpenCl.lib` file in the directory of the Intel OpenCL SDK (`C:\Program Files x86\IntelSWTools\OpenCL\sdk\lib\x64` which should contain the file `OpenCL.lib`)
7. Also add in `Linker` in the subsection `Input/Eingabe` the string `OpenCL.lib`
8. Change the project `.cpp` file to the following and run it to test if everything worked and list all found platforms/devices:

###### CMake ([01-intro-test-cmake](01-intro-test-cmake/))

**Prerequisites:**

- [MinGW](https://sourceforge.net/projects/mingw-w64/files/latest/download)
  - Select `x86_64` and `posix` threads
  - Install
  - Add `..ProgramFiles../mingw64/.../bin` to the Windows path
  - Copy in `..ProgramFiles../mingw64/.../bin` the file `mingw32-make.exe` and rename the copy into `make.exe`
- [CMake](https://cmake.org/download/)
  - Install
  - Add `...ProgramFiles.../cmake/bin` to the Windows path
- [Git](https://git-scm.com/downloads)
  - Install
- Restart your computer after all these steps

If you want to use CMake you can do the same by copying the code into a file called `main.cpp` and create the following `CMakeLists.txt` file:

```cmake
# Minimum CMake version
cmake_minimum_required (VERSION 3.14)

# Set name of project
project(main LANGUAGES CXX)

# Set C++ version
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include OpenCL headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Create the executable with the following source files:
add_executable(main main.cpp)

# Include the 64bit OpenCL.lib file
target_link_libraries(main ${CMAKE_SOURCE_DIR}/lib/x64/OpenCL.lib)
```

Also create for ease of use the file `build.sh`:

```sh
#! /usr/bin/env bash

# Check if cmake is installed
if ! [ -x "$(command -v cmake)" ]; then
  echo 'Error: cmake is not installed.' >&2
  exit 1
else
  echo -n "CMake version target: 3.14.2 => found: "
  cmake --version | head -n 1
fi

# Check if make is installed
if ! [ -x "$(command -v make)" ]; then
  echo 'Error: make is not installed.' >&2
  exit 1
else
  echo -n "Make version target: 4.2.1 => found: "
  make --version | head -n 1
fi

# OpenCL paths (Windows, Intel SDK)
OPENCL_SDK_PATH="/c/Program Files (x86)/IntelSWTools/OpenCL/sdk"
# Directory that contains the directory CL and in it the headers
OPENCL_INCLUDE_PATH="${OPENCL_SDK_PATH}/include"
# Directory which contains the /x64/OpenCL.lib file
OPENCL_LIBRARY_PATH="${OPENCL_SDK_PATH}/lib"

# Copy OpenCL
rm -rf include
rm -rf lib
cp -R "$OPENCL_INCLUDE_PATH" ./include
cp -R "$OPENCL_LIBRARY_PATH" ./lib

# Create directory for out of source build
rm -rf build
mkdir -p build
cd build

# Run CMake to create Makefile
echo "CMake: Windows MinGW -> MinGW Makefiles"
cmake .. -G "MinGW Makefiles" -Wdev -Wdeprecated -DCMAKE_SH="CMAKE_SH-NOTFOUND" -DCMAKE_BUILD_TYPE=Release

# Build executable
make

# Copy executable to dist directory
mkdir -p ../dist
cp main ../dist/main

# Run it with
# cd ..
# ./dist/main
```

Now enter into for example the GitBash (comes with any git installation on Windows) terminal:

```sh
./build.sh
```

And after that to run it:

```sh
./dist/main
```

## Terminology

- **Platform** (`cl::Platform`): The vendor specific OpenCL implementation (from Intel for example if you used the Setup>Windows>Intel guide)
-  **Context** (`cl::context`): The devices selected to work together with OpenCL
- **Devices**: The physical devices supporting to run OpenCL code (CPU/GPU/Accelerator)
- **Host**: The host is the client side calling code (CPU)
- **Kernel**: Blueprint function which will be run on the devices
- **Work item**: An instance of a computing unit executing a kernel
- **Work group**: A collection of work items executing the same kernel
- **Command queue**: The only way to communicate with a device, send it commands
- **Buffer**: Area of memory on the device
- **Memory**: Local, global, private, constant memory
  - ![Open CL memory model](hands-on-opencl-33-1024.jpg)
  - Image source: https://www.slideshare.net/vladimirstarostenkov/hands-on-opencl
